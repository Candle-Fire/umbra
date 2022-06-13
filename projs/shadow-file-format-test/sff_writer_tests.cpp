#include "pch.h"

import Shadow.FileFormat;

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

std::stringstream streamFrom(std::string str) {
	std::stringstream ss;
	ss << str;

	return ss;
}

TEST(EmptyFile, HasHeader) {

	std::stringstream ss = streamFrom(example_empty);

	//auto a = Shadow::SFF::SFFWriter::

	//auto assets = a->GetChildByIndex(0);

	//EXPECT_EQ(assets, nullptr);
}