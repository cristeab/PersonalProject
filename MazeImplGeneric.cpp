/*
 * MazeImplGeneric.cpp
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

TEST (test_maze, wall_test)
{
	Sp_Wall spWall(new Wall());
	EXPECT_EQ(spWall->Enter(), false);
}

TEST (test_maze, door_test)
{
	Sp_Room spRoom0(new Room(0));
	Sp_Room spRoom1(new Room(1));
	Sp_Door spDoor(new Door(spRoom0, spRoom1));
	EXPECT_EQ(spDoor->Enter(), false);
	EXPECT_EQ(spDoor->OtherSideFrom(spRoom0), spRoom1);
	EXPECT_EQ(spDoor->OtherSideFrom(spRoom1), spRoom0);
	Sp_Room spUnusedRoom;
	EXPECT_TRUE(spDoor->OtherSideFrom(spUnusedRoom).get() == NULL);
	spDoor->SetOpen();
	EXPECT_EQ(spDoor->Enter(), true);
}

TEST (test_maze, maze_test)
{
	Maze maze;
	Sp_Room spRoom0(new Room(0));
	Sp_Room spRoom1(new Room(1));
	Sp_Room spRoom2(new Room(2));
	maze.AddRoom(spRoom0);
	maze.AddRoom(spRoom1);
	maze.AddRoom(spRoom2);
	EXPECT_EQ(maze.RoomNo(0), spRoom0);
	EXPECT_EQ(maze.RoomNo(1), spRoom1);
	EXPECT_EQ(maze.RoomNo(2), spRoom2);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
