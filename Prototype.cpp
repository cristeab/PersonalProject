/*
 * Prototype.cpp
 *
 *  Created on: Dec 9, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
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
	Room() :
		roomNumber_(0)
	{
	}
	Room(int roomNo) :
		roomNumber_(roomNo)
	{
	}
	Room(const Room &other) :
		roomNumber_(other.roomNumber_)
	{
		for (int n = 0; n < 4; ++n)
		{
			spSides_[n] = other.spSides_[n];
		}
	}
	virtual Room* Clone() const
	{
		return new Room(*this);
	}
	void Initialize(int roomno)
	{
		roomNumber_ = roomno;
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
	Wall(const Wall &other)
	{
	}
	virtual Wall* Clone() const
	{
		return new Wall(*this);
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
	Door() :
		isOpen_(false)
	{
		for (int n = 0; n < 2; ++n)
		{
			spRoom_[n] = Sp_Room();//initialize with NULL pointers
		}
	}
	Door(const Sp_Room &ispRoom1, const Sp_Room &ispRoom2, bool isOpen = false) :
		isOpen_(isOpen)
	{
		spRoom_[0] = ispRoom1;
		spRoom_[1] = ispRoom2;
	}
	Door(const Door& other) :
		isOpen_(other.isOpen_)
	{
		spRoom_[0] = other.spRoom_[0];
		spRoom_[1] = other.spRoom_[1];
	}
	void Initialize(const Sp_Room &ispRoom1, const Sp_Room &ispRoom2, bool isOpen = false)
	{
		isOpen_ = isOpen;
		spRoom_[0] = ispRoom1;
		spRoom_[1] = ispRoom2;
	}
	Door* Clone() const
	{
		return new Door(*this);
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
	Maze(const Maze &other) :
		spRooms_(other.spRooms_)
	{
	}
	Maze* Clone() const
	{
		return new Maze(*this);
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
	BombedWall(const BombedWall &other) :
		isBombed_(other.isBombed_)
	{
	}
	virtual Wall* Clone() const
	{
		return new BombedWall(*this);
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
	RoomWithBomb() :
		Room(0), hasBomb_(true)
	{}
	RoomWithBomb(int roomNo) :
		Room(roomNo), hasBomb_(true)
	{}
	RoomWithBomb(const RoomWithBomb &other) :
		Room(other.roomNumber_), hasBomb_(other.hasBomb_)
	{
	}
	virtual Room* Clone() const
	{
		return new RoomWithBomb(*this);
	}
	void Initialize(int roomno)
	{
		Room::roomNumber_ = roomno;
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

//this is the Factory interface which also implements a concrete Factory
class MazeFactory
{
public:
	virtual Sp_Maze MakeMaze() const
	{
		return Sp_Maze(new Maze());
	}
	virtual Sp_Wall MakeWall() const
	{
		return Sp_Wall(new Wall());
	}
	virtual Sp_Room MakeRoom(int roomNo) const
	{
		return Sp_Room(new Room(roomNo));
	}
	virtual Sp_Door MakeDoor(const Sp_Room &r1, const Sp_Room &r2) const
	{
		return Sp_Door(new Door(r1, r2, true));
	}
};
typedef std::tr1::shared_ptr<MazeFactory> Sp_MazeFactory;

class MazePrototypeFactory : public MazeFactory
{
public:
	MazePrototypeFactory(Maze *ipMaze, Wall *ipWall, Room *ipRoom, Door *ipDoor) :
		pMaze_(ipMaze), pWall_(ipWall), pRoom_(ipRoom), pDoor_(ipDoor)
	{}
	Sp_Maze MakeMaze() const
	{
		return Sp_Maze(pMaze_->Clone());
	}
	Sp_Wall MakeWall() const
	{
		return Sp_Wall(pWall_->Clone());
	}
	Sp_Room MakeRoom(int roomno) const
	{
		Room *pRoom = pRoom_->Clone();
		pRoom->Initialize(roomno);
		return Sp_Room(pRoom);
	}
	Sp_Door MakeDoor(const Sp_Room ispRoom1, const Sp_Room ispRoom2) const
	{
		Door *pDoor = pDoor_->Clone();
		pDoor->Initialize(ispRoom1, ispRoom2);
		return Sp_Door(pDoor);
	}
private:
	Maze *pMaze_;
	Wall *pWall_;
	Room *pRoom_;
	Door *pDoor_;
};

Sp_Maze CreateMaze(MazeFactory &factory)
{
	Sp_Maze spMaze = factory.MakeMaze();
	Sp_Room spRoom1 = factory.MakeRoom(33);
	Sp_Room spRoom2 = factory.MakeRoom(34);
	Sp_Door spDoor = factory.MakeDoor(spRoom1, spRoom2);

	spMaze->AddRoom(spRoom1);
	spMaze->AddRoom(spRoom2);

	spRoom1->SetSide(North, factory.MakeWall());
	spRoom1->SetSide(East, spDoor);
	spRoom1->SetSide(South, factory.MakeWall());
	spRoom1->SetSide(West, factory.MakeWall());

	spRoom2->SetSide(North, factory.MakeWall());
	spRoom2->SetSide(East, factory.MakeWall());
	spRoom2->SetSide(South, factory.MakeWall());
	spRoom2->SetSide(West, spDoor);

	return spMaze;
}

TEST (maze_test_case, simple_maze_test)
{
	MazePrototypeFactory simpleMazeFactory(new Maze(), new Wall(), new Room(), new Door());
	Sp_Maze maze = CreateMaze(simpleMazeFactory);

	Sp_Room spRoom = maze->RoomNo(33);
	EXPECT_FALSE(spRoom.get() == NULL);
	EXPECT_TRUE(spRoom->Enter());

	Sp_MapSite spSite = spRoom->GetSide(North);
	EXPECT_FALSE(spSite->Enter());
	spSite = spRoom->GetSide(East);
	EXPECT_TRUE(spSite->Enter());
	spSite = spRoom->GetSide(West);
	EXPECT_FALSE(spSite->Enter());

	spRoom = maze->RoomNo(34);
	EXPECT_FALSE(spRoom.get() == NULL);
	EXPECT_TRUE(spRoom->Enter());

	spSite = spRoom->GetSide(North);
	EXPECT_FALSE(spSite->Enter());
	spSite = spRoom->GetSide(West);
	EXPECT_TRUE(spSite->Enter());
	spSite = spRoom->GetSide(East);
	EXPECT_FALSE(spSite->Enter());
}

TEST(maze_test_case, bombed_maze_test)
{
	MazePrototypeFactory bombedMazeFactory(new Maze(), new BombedWall(), new RoomWithBomb(), new Door());
	Sp_Maze maze = CreateMaze(bombedMazeFactory);

	Sp_Room spRoom = maze->RoomNo(33);
	EXPECT_FALSE(spRoom.get() == NULL);

	Sp_MapSite spSite = spRoom->GetSide(North);
	EXPECT_FALSE(spSite->Enter());
	spSite = spRoom->GetSide(East);
	EXPECT_TRUE(spSite->Enter());
	spSite = spRoom->GetSide(West);
	EXPECT_FALSE(spSite->Enter());

	EXPECT_TRUE(spRoom->Enter());
	EXPECT_TRUE(spSite->Enter());
	spSite = spRoom->GetSide(North);
	EXPECT_TRUE(spSite->Enter());

	spRoom = maze->RoomNo(34);
	EXPECT_FALSE(spRoom.get() == NULL);
	EXPECT_TRUE(spRoom->Enter());

	spSite = spRoom->GetSide(North);
	EXPECT_TRUE(spSite->Enter());
	spSite = spRoom->GetSide(West);
	EXPECT_TRUE(spSite->Enter());
	spSite = spRoom->GetSide(East);
	EXPECT_TRUE(spSite->Enter());
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
