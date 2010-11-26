/*
 * trim_string.cpp
 *
 *  Created on: Nov 26, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>

int trim_string(char *io_str)
{
	if (NULL == io_str)
	{
		return -1;
	}
	int str_len = strlen(io_str)+1;
	char *buf = new char[str_len];
	char *start_buf = buf;
	if (NULL == buf)
	{
		return -2;
	}
	std::copy(io_str, io_str+str_len, buf);
	bool found_space = false;
	while (*buf)
	{
		if (' ' == *buf)
		{
			if (buf == start_buf)
			{
				found_space = true;//rmove any starting spaces
			}
			if (found_space)
			{
				++buf;
				continue;
			} else
			{
				*io_str++ = *buf++;
				found_space = true;
			}
		} else
		{
			*io_str++ = *buf++;
			found_space = false;
		}
	}
	if (found_space && (*io_str != ' '))
	{
		--io_str;//remove any ending spaces
	}
	*io_str = '\0';

	delete[] start_buf;
	return 0;
}

TEST (test_trim, trim_test)
{
	EXPECT_EQ(trim_string(NULL), -1);

	char str1[] = "  ";
	EXPECT_EQ(trim_string(str1), 0);
	EXPECT_STREQ(str1, "");

	char str2[] = " ";
	EXPECT_EQ(trim_string(str2), 0);
	EXPECT_STREQ(str2, "");

	char str3[] = "abc  def";
	EXPECT_EQ(trim_string(str3), 0);
	EXPECT_STREQ(str3, "abc def");

	//any ending or starting spaces should be removed
	char str4[] = " abc  def   ";
	EXPECT_EQ(trim_string(str4), 0);
	EXPECT_STREQ(str4, "abc def");
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
