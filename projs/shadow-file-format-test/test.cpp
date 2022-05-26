#include "pch.h"

import Shadow.FileFormat;

std::string example_str = "ShadowFileFormat_1_0_0 \n\
Assets:{ \
	9:textures / cube_maps / ame_ash / ashcanyon.sff, \
	10 : textures / models / checker_board.sff, \
	12 : shaders / skybox / skybox.sff, \
} \
";


TEST(TestCaseName, TestName) {

	std::stringstream ss;
	ss << example_str;

	auto a = Shadow::SFF::SFFParser::ReadFromStream(ss);

	auto assets = a->GetChildByIndex(0);

	EXPECT_STREQ(assets->name.c_str(), "Assets");
}