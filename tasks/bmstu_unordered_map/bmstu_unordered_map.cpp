#include <bmstu_unordered_map.h>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

using namespace std::string_literals;

TEST(UnorderedMapTests, init)
{
	{
		bmstu::unordered_map<int, int> empty_map;
		ASSERT_EQ(empty_map.size(), 0u);
		ASSERT_TRUE(empty_map.empty());
		ASSERT_EQ(empty_map.bucket_count(), 16u);
	}

	{
		bmstu::unordered_map<std::string, int> empty_string_map;
		ASSERT_EQ(empty_string_map.size(), 0u);
		ASSERT_TRUE(empty_string_map.empty());
	}
}

TEST(UnorderedMapTests, insert_and_find)
{
	bmstu::unordered_map<int, std::string> map;

	map.insert(1, "one");
	map.insert(2, "two");
	map.insert(3, "three");

	ASSERT_EQ(map.size(), 3u);
	ASSERT_FALSE(map.empty());

	auto it = map.find(2);
	ASSERT_NE(it, map.end());
	ASSERT_EQ(it->first, 2);
	ASSERT_EQ(it->second, "two");

	it = map.find(4);
	ASSERT_EQ(it, map.end());

	ASSERT_TRUE(map.contains(1));
	ASSERT_TRUE(map.contains(3));
	ASSERT_FALSE(map.contains(5));
}

TEST(UnorderedMapTests, operator_brackets)
{
	bmstu::unordered_map<std::string, int> map;

	map["apple"] = 5;
	map["banana"] = 7;
	map["orange"] = 10;

	ASSERT_EQ(map.size(), 3u);
	ASSERT_EQ(map["apple"], 5);
	ASSERT_EQ(map["banana"], 7);
	ASSERT_EQ(map["orange"], 10);

	map["apple"] = 15;
	ASSERT_EQ(map["apple"], 15);

	ASSERT_EQ(map["grape"], int());
	ASSERT_EQ(map.size(), 4u);
	ASSERT_TRUE(map.contains("grape"));
}

TEST(UnorderedMapTests, erase)
{
	bmstu::unordered_map<int, std::string> map;

	map.insert(1, "one");
	map.insert(2, "two");
	map.insert(3, "three");
	map.insert(4, "four");

	ASSERT_EQ(map.size(), 4u);

	map.erase(2);
	ASSERT_EQ(map.size(), 3u);
	ASSERT_FALSE(map.contains(2));
	ASSERT_TRUE(map.contains(1));
	ASSERT_TRUE(map.contains(3));
	ASSERT_TRUE(map.contains(4));

	map.erase(5);
	ASSERT_EQ(map.size(), 3u);

	map.erase(1);
	map.erase(3);
	map.erase(4);
	ASSERT_TRUE(map.empty());
	ASSERT_EQ(map.size(), 0u);
}

TEST(UnorderedMapTests, contains)
{
	bmstu::unordered_map<int, std::string> map;

	map[10] = "ten";
	map[20] = "twenty";
	map[30] = "thirty";

	ASSERT_TRUE(map.contains(10));
	ASSERT_TRUE(map.contains(20));
	ASSERT_TRUE(map.contains(30));
	ASSERT_FALSE(map.contains(40));
	ASSERT_FALSE(map.contains(0));
}

TEST(UnorderedMapTests, clear)
{
	bmstu::unordered_map<int, int> map;

	for (int i = 0; i < 100; ++i)
	{
		map[i] = i * i;
	}

	ASSERT_EQ(map.size(), 100u);

	map.clear();

	ASSERT_EQ(map.size(), 0u);
	ASSERT_TRUE(map.empty());
	ASSERT_FALSE(map.contains(0));
	ASSERT_FALSE(map.contains(50));
	ASSERT_FALSE(map.contains(100));
}

TEST(UnorderedMapTests, copy_constructor)
{
	bmstu::unordered_map<int, std::string> map1;

	map1[1] = "one";
	map1[2] = "two";
	map1[3] = "three";

	bmstu::unordered_map<int, std::string> map2(map1);

	ASSERT_EQ(map2.size(), map1.size());
	ASSERT_EQ(map2[1], "one");
	ASSERT_EQ(map2[2], "two");
	ASSERT_EQ(map2[3], "three");

	map2[2] = "TWO";
	ASSERT_EQ(map1[2], "two");
	ASSERT_EQ(map2[2], "TWO");

	map2.insert(4, "four");
	ASSERT_EQ(map1.size(), 3u);
	ASSERT_EQ(map2.size(), 4u);
}

TEST(UnorderedMapTests, copy_assignment)
{
	bmstu::unordered_map<int, int> map1;
	bmstu::unordered_map<int, int> map2;

	for (int i = 0; i < 10; i++)
	{
		map1[i] = i * 10;
	}

	map2 = map1;

	ASSERT_EQ(map2.size(), map1.size());
	for (int i = 0; i < 10; i++)
	{
		ASSERT_EQ(map2[i], i * 10);
	}
}

TEST(UnorderedMapTests, move_constructor)
{
	bmstu::unordered_map<int, std::string> map1;

	map1[1] = "one";
	map1[2] = "two";
	map1[3] = "three";

	size_t old_size = map1.size();

	bmstu::unordered_map<int, std::string> map2(std::move(map1));

	ASSERT_EQ(map2.size(), old_size);
	ASSERT_TRUE(map1.empty());
	ASSERT_EQ(map2[1], "one");
	ASSERT_EQ(map2[2], "two");
	ASSERT_EQ(map2[3], "three");
}

TEST(UnorderedMapTests, move_assignment)
{
	bmstu::unordered_map<int, int> map1;
	bmstu::unordered_map<int, int> map2;

	for (int i = 0; i < 5; ++i)
	{
		map1[i] = i;
	}

	map2 = std::move(map1);

	ASSERT_EQ(map2.size(), 5u);
	for (int i = 0; i < 5; ++i)
	{
		ASSERT_EQ(map2[i], i);
	}
}

TEST(UnorderedMapTests, swap)
{
	bmstu::unordered_map<int, std::string> map1;
	bmstu::unordered_map<int, std::string> map2;

	map1[1] = "one";
	map1[2] = "two";

	map2[3] = "three";
	map2[4] = "four";
	map2[5] = "five";

	auto old_size1 = map1.size();
	auto old_size2 = map2.size();

	map1.swap(map2);

	ASSERT_EQ(map1.size(), old_size2);
	ASSERT_EQ(map2.size(), old_size1);

	ASSERT_TRUE(map1.contains(3));
	ASSERT_TRUE(map1.contains(4));
	ASSERT_TRUE(map1.contains(5));
	ASSERT_TRUE(map2.contains(1));
	ASSERT_TRUE(map2.contains(2));
}

TEST(UnorderedMapTests, iteration)
{
	bmstu::unordered_map<int, int> map;

	for (int i = 0; i < 10; i++)
	{
		map[i] = i * i;
	}

	int count = 0;
	for (const auto& pair : map)
	{
		ASSERT_EQ(pair.second, pair.first * pair.first);
		++count;
	}
	ASSERT_EQ(count, 10);

	const auto& const_map = map;
	count = 0;
	for (auto it = const_map.cbegin(); it != const_map.cend(); it++)
	{
		ASSERT_EQ(it->second, it->first * it->first);
		++count;
	}
	ASSERT_EQ(count, 10);
}

TEST(UnorderedMapTests, equality_operator)
{
	bmstu::unordered_map<int, std::string> map1;
	bmstu::unordered_map<int, std::string> map2;
	bmstu::unordered_map<int, std::string> map3;

	map1[1] = "one";
	map1[2] = "two";
	map1[3] = "three";

	map2[1] = "one";
	map2[2] = "two";
	map2[3] = "three";

	map3[1] = "one";
	map3[2] = "TWO";
	map3[3] = "three";

	ASSERT_TRUE(map1 == map2);
	ASSERT_FALSE(map1 == map3);
	ASSERT_FALSE(map2 == map3);

	map2[4] = "four";
	ASSERT_FALSE(map1 == map2);
}

TEST(UnorderedMapTests, unequality_operator)
{
	bmstu::unordered_map<int, int> map1;
	bmstu::unordered_map<int, int> map2;

	map1[1] = 1;
	map1[2] = 2;

	map2[1] = 1;
	map2[2] = 2;

	ASSERT_FALSE(map1 != map2);

	map2[3] = 3;
	ASSERT_TRUE(map1 != map2);
}

TEST(UnorderedMapTests, initializer_list)
{
	bmstu::unordered_map<int, std::string> map = {
		{1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}};

	ASSERT_EQ(map.size(), 4u);
	ASSERT_EQ(map[1], "one");
	ASSERT_EQ(map[2], "two");
	ASSERT_EQ(map[3], "three");
	ASSERT_EQ(map[4], "four");
}

TEST(UnorderedMapTests, rehash)
{
	bmstu::unordered_map<int, int> map;

	size_t initial_buckets = map.bucket_count();
	ASSERT_EQ(initial_buckets, 16u);

	for (int i = 0; i < 100; i++)
	{
		map[i] = i;
	}
	ASSERT_GT(map.bucket_count(), initial_buckets);
	ASSERT_EQ(map.size(), 100u);

	for (int i = 0; i < 100; i++)
	{
		ASSERT_TRUE(map.contains(i));
		ASSERT_EQ(map[i], i);
	}
}

TEST(UnorderedMapTests, update_value)
{
	bmstu::unordered_map<int, int> map;

	map[5] = 10;
	ASSERT_EQ(map[5], 10);

	map.insert(5, 20);
	ASSERT_EQ(map[5], 20);

	map[5] = 30;
	ASSERT_EQ(map[5], 30);
}

TEST(UnorderedMapTests, const_operations)
{
	bmstu::unordered_map<int, int> map;
	map[1] = 100;
	map[2] = 200;
	map[3] = 300;

	const auto& const_map = map;

	ASSERT_EQ(const_map.size(), 3u);
	ASSERT_FALSE(const_map.empty());

	auto it = const_map.find(2);
	ASSERT_NE(it, const_map.end());
	ASSERT_EQ(it->second, 200);

	ASSERT_TRUE(const_map.contains(3));
	ASSERT_FALSE(const_map.contains(4));

	auto cit = const_map.cbegin();
	ASSERT_EQ(cit->first, 1);
	ASSERT_EQ(cit->second, 100);
}

TEST(UnorderedMapTests, iterator_invalidation)
{
	bmstu::unordered_map<int, int> map;

	for (int i = 0; i < 20; ++i)
	{
		map[i] = i * 2;
	}

	auto it = map.find(10);
	ASSERT_NE(it, map.end());
	ASSERT_EQ(it->second, 20);

	for (int i = 20; i < 100; ++i)
	{
		map[i] = i * 2;
	}

	it = map.find(10);
	ASSERT_NE(it, map.end());
	ASSERT_EQ(it->second, 20);
}

TEST(UnorderedMapTests, string_keys)
{
	bmstu::unordered_map<std::string, int> map;

	map["one"] = 1;
	map["two"] = 2;
	map["three"] = 3;

	ASSERT_EQ(map["one"], 1);
	ASSERT_EQ(map["two"], 2);
	ASSERT_EQ(map["three"], 3);

	map["two"] = 22;
	ASSERT_EQ(map["two"], 22);

	ASSERT_TRUE(map.contains("one"));
	ASSERT_FALSE(map.contains("four"));
}

TEST(UnorderedMapTests, large_data)
{
	bmstu::unordered_map<int, int> map;
	const int N = 10000;

	for (int i = 0; i < N; ++i)
	{
		map[i] = i * i;
	}

	ASSERT_EQ(map.size(), N);

	for (int i = 0; i < N; ++i)
	{
		ASSERT_TRUE(map.contains(i));
		ASSERT_EQ(map[i], i * i);
	}

	for (int i = 0; i < N; i += 2)
	{
		map.erase(i);
	}

	ASSERT_EQ(map.size(), N / 2);

	for (int i = 0; i < N; ++i)
	{
		if (i % 2 == 0)
		{
			ASSERT_FALSE(map.contains(i));
		}
		else
		{
			ASSERT_TRUE(map.contains(i));
			ASSERT_EQ(map[i], i * i);
		}
	}
}

TEST(UnorderedMapTests, empty_map_operations)
{
	bmstu::unordered_map<int, int> map;

	ASSERT_EQ(map.find(42), map.end());
	ASSERT_FALSE(map.contains(42));
	ASSERT_EQ(map.size(), 0u);
	ASSERT_TRUE(map.empty());

	map.erase(42);
	map[42] = 100;
	ASSERT_EQ(map.size(), 1u);
	ASSERT_EQ(map[42], 100);
}

TEST(UnorderedMapTests, empty_map_operations1)
{
	bmstu::unordered_map<int, int> map;

	ASSERT_EQ(map.find(95), map.end());
	ASSERT_FALSE(map.contains(95));
	ASSERT_EQ(map.size(), 0u);
	ASSERT_TRUE(map.empty());

	map.erase(95);

	map[95] = 100;
	ASSERT_EQ(map.size(), 1u);
	ASSERT_EQ(map[95], 100);
}

TEST(UnorderedMapTests, empty_map_operations_1)
{
	bmstu::unordered_map<int, int> map;

	ASSERT_EQ(map.find(100), map.end());
	ASSERT_FALSE(map.contains(100));
	ASSERT_EQ(map.size(), 0u);
	ASSERT_TRUE(map.empty());

	map.erase(100);

	map[100] = 100;
	ASSERT_EQ(map.size(), 1u);
	ASSERT_EQ(map[100], 100);
}

TEST(UnorderedMapTests, at)
{
	bmstu::unordered_map<int, int> map;

	ASSERT_THROW(map.at(100), std::out_of_range);

	map[100] = 500;

	ASSERT_EQ(map.at(100), 500);

	const auto& map2 = map;

	ASSERT_EQ(map2.at(100), 500);

	auto map5 = std::move(map);

	ASSERT_EQ(map5.at(100), 500);
}