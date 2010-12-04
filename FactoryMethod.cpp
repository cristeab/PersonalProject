#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cstdlib>

#include <vector>
#include <tr1/memory>

enum Direction {North, South, East, West};

class MapSite
{
public:
	virtual bool Enter() = 0;
	virtual ~MapSite()
	{
	}
};
typedef std::tr1::shared_ptr<MapSite> Sp_MapSite;

class Room : public MapSite
{
public:
	Room(int roomNo) :
		roomNumber_(roomNo)
	{
	}
	Sp_MapSite GetSide(Direction dir) const
	{
		return Sp_MapSite(spSides_[dir]);
	}
	void SetSide(Direction dir, const Sp_MapSite &ispSide)
	{
		spSides_[dir] = ispSide;
	}
	int GetRoomNumber() const
	{
		return roomNumber_;
	}
	virtual bool Enter()
	{
		//std::cout << "Entering room no " << roomNumber_ << std::endl;
		return true;
	}
protected:
	int roomNumber_;
	Sp_MapSite spSides_[4];
};
typedef std::tr1::shared_ptr<Room> Sp_Room;

class Wall : public MapSite
{
public:
	Wall()
	{
	}
	virtual bool Enter()
	{
		//std::cout << "Cannot enter into a wall" << std::endl;
		return false;
	}
};
typedef std::tr1::shared_ptr<Wall> Sp_Wall;

class Door : public MapSite
{
public:
	Door(const Sp_Room &ispRoom1, const Sp_Room &ispRoom2, bool isOpen = false) :
		isOpen_(isOpen)
	{
		spRoom_[0] = ispRoom1;
		spRoom_[1] = ispRoom2;
	}
	Sp_Room OtherSideFrom(const Sp_Room &ispRoom)
	{
		if (ispRoom == spRoom_[0])
		{
			return spRoom_[1];
		} else if (ispRoom == spRoom_[1])
		{
			return spRoom_[0];
		}
		return Sp_Room();
	}
	void SetOpen()
	{
		isOpen_ = true;
	}
	virtual bool Enter()
	{
		return isOpen_;
	}
private:
	bool isOpen_;
	Sp_Room spRoom_[2];
};
typedef std::tr1::shared_ptr<Door> Sp_Door;

class Maze
{
public:
	Maze()
	{
	}
	void AddRoom(Sp_Room ispRoom)
	{
		spRooms_.push_back(ispRoom);
	}
	Sp_Room RoomNo(int roomNo) const
	{
		for (unsigned int n = 0; n < spRooms_.size(); ++n)
		{
			if (spRooms_[n]->GetRoomNumber() == roomNo)
			{
				return spRooms_[n];
			}
		}
		return Sp_Room();
	}
private:
	std::vector<Sp_Room> spRooms_;
};
typedef std::tr1::shared_ptr<Maze> Sp_Maze;

class BombedWall : public Wall
{
public:
	BombedWall() :
		isBombed_(false)
	{
	}
	void BombWall()
	{
		isBombed_ = true;
	}
	virtual bool Enter()
	{
		//std::cout << "Cannot enter into a wall" << std::endl;
		return isBombed_;
	}
private:
	bool isBombed_;
};
typedef std::tr1::shared_ptr<BombedWall> Sp_BombedWall;

class RoomWithBomb : public Room
{
public:
	RoomWithBomb(int roomNo) :
		Room(roomNo), hasBomb_(true)
	{
	}
	void RemoveBomb()
	{
		hasBomb_ = false;
	}
	virtual bool Enter()
	{
		for (int n = 0; n < 4; ++n)
		{
			if (BombedWall *pBombedWall = dynamic_cast<BombedWall*>(spSides_[n].get()))
			{
				pBombedWall->BombWall();
			} else if (Door* pDoor = dynamic_cast<Door*>(spSides_[n].get()))
			{
				pDoor->SetOpen();
			} else
			{
				//std::cerr << "Cannot cast the MapSites" << std::endl;
				return false;
			}
		}
		return true;
	}
private:
	bool hasBomb_;
};
typedef std::tr1::shared_ptr<RoomWithBomb> Sp_RoomWithBomb;

class StandardMazeGame
{
protected:
	virtual Sp_Maze MakeMaze() const
	{
		return Sp_Maze(new Maze());
	}
	virtual Sp_Room MakeRoom(int roomno) const
	{
		return Sp_Room(new Room(roomno));
	}
	virtual Sp_Wall MakeWall() const
	{
		return Sp_Wall(new Wall());
	}
	virtual Sp_Door MakeDoor(const Sp_Room &ispRoom1, const Sp_Room &ispRoom2, bool isOpen = false) const
	{
		return Sp_Door(new Door(ispRoom1, ispRoom2, isOpen));
	}
public:
	Sp_Maze CreateMaze()
	{
		Sp_Maze spMaze = MakeMaze();

		Sp_Room spRoom1 = MakeRoom(1);
		Sp_Room spRoom2 = MakeRoom(2);
		Sp_Door spDoor = MakeDoor(spRoom1, spRoom2, true);
		spRoom1->SetSide(North, MakeWall());
		spRoom1->SetSide(East, spDoor);
		spRoom1->SetSide(South, MakeWall());
		spRoom1->SetSide(West, MakeWall());

		spRoom2->SetSide(North, MakeWall());
		spRoom2->SetSide(East, MakeWall());
		spRoom2->SetSide(South, MakeWall());
		spRoom2->SetSide(West, spDoor);
		spMaze->AddRoom(spRoom1);
		spMaze->AddRoom(spRoom2);

		return spMaze;
	}
};

class BombedMazeGame : public StandardMazeGame
{
protected:
	virtual Sp_Room MakeRoom(int roomno) const
	{
		return Sp_RoomWithBomb(new RoomWithBomb(roomno));
	}
	virtual Sp_Wall MakeWall() const
	{
		return Sp_BombedWall(new BombedWall());
	}
};

class MazeTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(MazeTest);
	CPPUNIT_TEST(TestStandardMaze);
	CPPUNIT_TEST(TestBombedMaze);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp()
	{
	}
	void tearDown()
	{
	}
	void TestStandardMaze()
	{
		StandardMazeGame maze;
		Sp_Maze spMaze = maze.CreateMaze();

		Sp_Room spRoom = spMaze->RoomNo(1);

		CPPUNIT_ASSERT(spRoom.get() != NULL);
		CPPUNIT_ASSERT(spRoom->GetRoomNumber() == 1);
		CPPUNIT_ASSERT(spRoom->Enter() == true);

		Sp_MapSite spSite = spRoom->GetSide(West);
		CPPUNIT_ASSERT(spSite->Enter() == false);
		spSite = spRoom->GetSide(East);
		CPPUNIT_ASSERT(spSite->Enter() == true);

		spRoom = spMaze->RoomNo(2);
		CPPUNIT_ASSERT(spRoom.get() != NULL);
		CPPUNIT_ASSERT(spRoom->GetRoomNumber() == 2);
		CPPUNIT_ASSERT(spRoom->Enter() == true);

		spSite = spRoom->GetSide(West);
		CPPUNIT_ASSERT(spSite->Enter() == true);
		spSite = spRoom->GetSide(East);
		CPPUNIT_ASSERT(spSite->Enter() == false);
	}
	void TestBombedMaze()
	{
		BombedMazeGame maze;
		Sp_Maze spMaze = maze.CreateMaze();

		Sp_Room spRoom = spMaze->RoomNo(1);

		CPPUNIT_ASSERT(spRoom.get() != NULL);
		CPPUNIT_ASSERT(spRoom->GetRoomNumber() == 1);
		CPPUNIT_ASSERT(spRoom->Enter() == true);//the room has exploded

		Sp_MapSite spSite = spRoom->GetSide(West);
		CPPUNIT_ASSERT(spSite->Enter() == true);
		spSite = spRoom->GetSide(East);
		CPPUNIT_ASSERT(spSite->Enter() == true);

		spRoom = spMaze->RoomNo(2);
		CPPUNIT_ASSERT(spRoom.get() != NULL);
		CPPUNIT_ASSERT(spRoom->GetRoomNumber() == 2);
		CPPUNIT_ASSERT(spRoom->Enter() == true);

		spSite = spRoom->GetSide(West);
		CPPUNIT_ASSERT(spSite->Enter() == true);
		spSite = spRoom->GetSide(East);
		CPPUNIT_ASSERT(spSite->Enter() == true);
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(MazeTest);

int main()
{
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	return runner.run()?EXIT_SUCCESS:EXIT_FAILURE;
}
