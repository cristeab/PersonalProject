/*
 * AbstractFactory.cpp
 *
 *  Created on: Nov 27, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <iostream>
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

class BombedMazeFactory : public MazeFactory
{
public:
	virtual Sp_Wall MakeWall() const
	{
		return Sp_Wall(new BombedWall());
	}
	virtual Sp_Room MakeRoom(int roomNo) const
	{
		return Sp_Room(new RoomWithBomb(roomNo));
	}
};
typedef std::tr1::shared_ptr<BombedMazeFactory> Sp_BombedMazeFactory;

TEST (test_maze, maze_test)
{
	MazeFactory factory;
	Sp_Maze spMaze = CreateMaze(factory);

	Sp_Room spRoom = spMaze->RoomNo(33);
	EXPECT_TRUE(spRoom.get() != NULL);
	EXPECT_EQ(spRoom->GetRoomNumber(), 33);

	Sp_MapSite spSite = spRoom->GetSide(East);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	spSite = spRoom->GetSide(North);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(South);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(West);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spRoom = spMaze->RoomNo(34);
	EXPECT_TRUE(spRoom.get() != NULL);
	EXPECT_EQ(spRoom->GetRoomNumber(), 34);

	spSite = spRoom->GetSide(East);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(North);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(South);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(West);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);
}

TEST (test_maze, bombed_maze_test)
{
	BombedMazeFactory factory;
	Sp_Maze spMaze = CreateMaze(factory);

	Sp_Room spRoom = spMaze->RoomNo(33);
	EXPECT_TRUE(spRoom.get() != NULL);
	EXPECT_EQ(spRoom->GetRoomNumber(), 33);

	Sp_MapSite spSite = spRoom->GetSide(East);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	spSite = spRoom->GetSide(North);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(South);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(West);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spRoom = spMaze->RoomNo(34);
	EXPECT_TRUE(spRoom.get() != NULL);
	EXPECT_EQ(spRoom->GetRoomNumber(), 34);

	spSite = spRoom->GetSide(East);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(North);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(South);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), false);

	spSite = spRoom->GetSide(West);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	//enter into a bombed room
	EXPECT_TRUE(spRoom->Enter());

	spSite = spRoom->GetSide(East);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	spSite = spRoom->GetSide(North);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	spSite = spRoom->GetSide(South);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	spSite = spRoom->GetSide(West);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	//same should happen for the other room
	spRoom = spMaze->RoomNo(33);
	EXPECT_TRUE(spRoom.get() != NULL);
	EXPECT_EQ(spRoom->GetRoomNumber(), 33);
	EXPECT_TRUE(spRoom->Enter());

	spSite = spRoom->GetSide(East);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	spSite = spRoom->GetSide(North);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	spSite = spRoom->GetSide(South);
	EXPECT_TRUE(spSite.get() != NULL);
	EXPECT_EQ(spSite->Enter(), true);

	spSite = spRoom->GetSide(West);
	EXPECT_TRUE(spSite != NULL);
	EXPECT_EQ(spSite->Enter(), true);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
