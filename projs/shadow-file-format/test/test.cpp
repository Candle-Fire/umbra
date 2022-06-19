#include "gtest/gtest.h"

#include <string>
#include <sstream>
#include "SFFParser.h"

//import Shadow.FileFormat;

std::string example_empty = "ShadowFileFormat_1_0_0";

std::string example_simple = "ShadowFileFormat_1_0_0 \n\
Assets:{ \
	9:Content_9, \
	10 : Content_10, \
	11: Content_11, \
}, \
";

std::string example_multi_root = 
"ShadowFileFormat_1_0_0 \n	\
Assets:{					\
	9:Content_9,			\
},							\
Texture:{					\
	texture:checker_board.png,	\
},							\
";


std::string example_multi_level = "ShadowFileFormat_1_0_0 \n\
Assets:{ \
	9: { \
		\
	}, \
}, \
";



std::stringstream streamFrom(std::string str) {
	std::stringstream ss;
	ss << str;

	return ss;
}

TEST(EmptyFile, HasHeader) {

	std::stringstream ss = streamFrom(example_empty);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByIndex(0);

	EXPECT_EQ(assets, nullptr);
}


TEST(SimpleFile, SingleRoot) {

	std::stringstream ss = streamFrom(example_simple);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	EXPECT_EQ(a->children.size(), 1);


	auto assets = a->GetChildByIndex(0);

	EXPECT_NE(assets, nullptr);
	EXPECT_STREQ(assets->name.c_str(), "Assets");
}

TEST(SimpleFile, RootByName) {

	std::stringstream ss = streamFrom(example_simple);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByName("Assets");

	EXPECT_NE(assets, nullptr);
	EXPECT_STREQ(assets->name.c_str(), "Assets");
}

TEST(SimpleFile, SubChildren) {

	std::stringstream ss = streamFrom(example_simple);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByIndex(0);

	EXPECT_EQ(assets->children.size(), 3);
}

TEST(SimpleFile, SubChildrenExist) {

	std::stringstream ss = streamFrom(example_simple);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByIndex(0);
	

	for (size_t i = 0; i < 3; i++)
	{
		auto i9 = assets->GetChildByName(std::to_string(9+i));
		EXPECT_NE(i9, nullptr);
	}

}

TEST(SimpleFile, SubChildrenContent) {

	std::stringstream ss = streamFrom(example_simple);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByIndex(0);


	for (size_t i = 0; i < 3; i++)
	{
		std::string name = std::to_string(9 + i);
		auto i9 = assets->GetChildByName(name);
		EXPECT_EQ(i9->value, "Content_"+ name);
	}

}


TEST(MultiLevel, SinlgeRootExists) {

	std::stringstream ss = streamFrom(example_multi_level);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	EXPECT_EQ(a->children.size(), 1);
}

TEST(MultiLevel, SecondLevelExists) {

	std::stringstream ss = streamFrom(example_multi_level);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto asset = a->GetChildByIndex(0);

	EXPECT_EQ(asset->children.size(), 1);
}

TEST(MultiLevel, BlockTagIsCorrect) {

	std::stringstream ss = streamFrom(example_multi_level);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByIndex(0);
	EXPECT_EQ(assets->isBlock, true);

	auto element = assets->GetChildByIndex(0);
	EXPECT_EQ(element->isBlock, true);
}

TEST(MultiLevel, LevelsHaveCorrectName) {

	std::stringstream ss = streamFrom(example_multi_level);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByIndex(0);
	EXPECT_EQ(assets->name, "Assets");

	auto element = assets->GetChildByIndex(0);
	EXPECT_EQ(element->name, "9");
}


TEST(MultiRoot, RootsExist) {

	std::stringstream ss = streamFrom(example_multi_root);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	EXPECT_EQ(a->children.size(), 2);
}

TEST(MultiRoot, RootsHaveCorrectName) {

	std::stringstream ss = streamFrom(example_multi_root);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByName("Assets");

	EXPECT_NE(assets, nullptr);
	EXPECT_EQ(assets->name, "Assets");

	auto texture = a->GetChildByName("Texture");

	EXPECT_NE(texture, nullptr);
	EXPECT_EQ(texture->name, "Texture");
}

std::string example_multi_level_content = "ShadowFileFormat_1_0_0 \n\
Assets:{ \
	a: { \
		0: ContentContent0,	\
		1: ContentContent1,	\
		2: ContentContent2,	\
	},						\
	b: ContentB,			\
}, \
";

TEST(MultiLevelContent, RootsHaveCorrectName) {

	std::stringstream ss = streamFrom(example_multi_level_content);

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByName("Assets");

	EXPECT_NE(assets, nullptr);
	EXPECT_EQ(assets->name, "Assets");
	EXPECT_EQ(assets->isBlock, true);

	auto first = assets->GetChildByName("a");

	EXPECT_NE(first, nullptr);
	EXPECT_EQ(first->name, "a");
	EXPECT_EQ(first->isBlock, true);

	for (size_t i = 0; i < 3; i++)
	{
		std::string name = std::to_string(i);
		auto element = first->GetChildByName(name);

		EXPECT_NE(element, nullptr);
		EXPECT_EQ(element->name, name);
		EXPECT_EQ(element->isBlock, false);

		EXPECT_EQ(element->value, "ContentContent" + name);
	}



	auto second = assets->GetChildByName("b");

	EXPECT_NE(second, nullptr);
	EXPECT_EQ(second->name, "b");
	EXPECT_EQ(second->isBlock, false);
	EXPECT_EQ(second->value, "ContentB");
}