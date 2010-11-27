/*
 * MazeImplGeneric.cpp
 *
 *  Created on: Nov 27, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <iostream>
#include <vector>

enum Direction {North, South, East, West};

class MapSite
{
public:
	virtual bool Enter() = 0;
	virtual ~MapSite()
	{
	}
};

class Room : public MapSite
{
public:
	Room(int roomNo) :
		roomNumber_(roomNo)
	{
		for (int n = 0; n < 4; ++n)
		{
			sides_[n] = NULL;
		}
	}
	MapSite* GetSide(Direction dir) const
	{
		return sides_[dir];
	}
	bool SetSide(Direction dir, MapSite *ipSide)
	{
		if (NULL == ipSide)
		{
			return false;
		}
		sides_[dir] = ipSide;
		return true;
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
private:
	int roomNumber_;
	MapSite *sides_[4];
};

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

class Door : public MapSite
{
public:
	Door(Room *ipRoom1 = NULL, Room *ipRoom2 = NULL) :
		isOpen_(false)
	{
		pRoom_[0] = ipRoom1;
		pRoom_[1] = ipRoom2;
	}
	Room* OtherSideFrom(Room *ipRoom)
	{
		if (ipRoom == pRoom_[0])
		{
			return pRoom_[1];
		} else if (ipRoom == pRoom_[1])
		{
			return pRoom_[0];
		}
		return NULL;
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
	Room *pRoom_[2];
};

class Maze
{
public:
	Maze()
	{
	}
	void AddRoom(Room *ipRoom)
	{
		rooms_.push_back(ipRoom);
	}
	Room* RoomNo(int roomNo) const
	{
		for (unsigned int n = 0; n < rooms_.size(); ++n)
		{
			if (rooms_[n]->GetRoomNumber() == roomNo)
			{
				return rooms_[n];
			}
		}
		return NULL;
	}
private:
	std::vector<Room*> rooms_;
};

TEST (test_maze, room_test)
{
	Room *pRoom = new Room(0);
	MapSite *pWall = new Wall();
	EXPECT_EQ(pRoom->SetSide(North, pWall), true);
	EXPECT_EQ(pRoom->GetSide(North), pWall);
	EXPECT_EQ(pRoom->GetRoomNumber(), 0);
	EXPECT_EQ(pRoom->Enter(), true);
	delete pRoom;
	delete pWall;
}

TEST (test_maze, wall_test)
{
	Wall *pWall = new Wall();
	EXPECT_EQ(pWall->Enter(), false);
}

TEST (test_maze, door_test)
{
	Room *pRoom0 = new Room(0);
	Room *pRoom1 = new Room(1);
	Door *pDoor = new Door(pRoom0, pRoom1);
	EXPECT_EQ(pDoor->Enter(), false);
	EXPECT_EQ(pDoor->OtherSideFrom(pRoom0), pRoom1);
	EXPECT_EQ(pDoor->OtherSideFrom(pRoom1), pRoom0);
	Room *pNullRoom = NULL;
	EXPECT_EQ(pDoor->OtherSideFrom(NULL), pNullRoom);
	pDoor->SetOpen();
	EXPECT_EQ(pDoor->Enter(), true);
	delete pRoom0;
	delete pRoom1;
	delete pDoor;
}

TEST (test_maze, maze_test)
{
	Maze maze;
	Room room0(0);
	Room room1(1);
	Room room2(2);
	maze.AddRoom(&room0);
	maze.AddRoom(&room1);
	maze.AddRoom(&room2);
	EXPECT_EQ(maze.RoomNo(0), &room0);
	EXPECT_EQ(maze.RoomNo(1), &room1);
	EXPECT_EQ(maze.RoomNo(2), &room2);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
