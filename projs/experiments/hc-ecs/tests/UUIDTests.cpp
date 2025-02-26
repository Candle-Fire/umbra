#include <UUID.h>
#include <catch2/catch_all.hpp>

sonic::UUID testUUID1(0xFFBC5192D8A1EF24, 0x038568C2D9EBCF15);
std::string testUUID1Str("FFBC5192-D8A1-EF24-0385-68C2D9EBCF15");

std::string invalidUUID( "FFBC5192D8A1EF24038568C2D9EBCF15");
std::string pathological("THISISTHIRTYSIXCHARACTERSLONGTOO");

SCENARIO("UUIDs are 16 bytes long") {
  GIVEN("an empty UUID") {
    sonic::UUID test;
    THEN("uuid has correct size") {
      REQUIRE(sizeof(test) == 16);
    }
  }
}

SCENARIO("Creating a valid UUID from bytes") {
  GIVEN("a uuid created with value FFBC5192-D8A1-EF24-0385-68C2D9EBCF15 by four uint16_ts") {
    sonic::UUID test(0xFFBC5192, 0xD8A1EF24, 0x038568C2, 0xD9EBCF15);
    THEN("uuid matches the test UUID") {
      REQUIRE(test == testUUID1);
    }
    AND_THEN("uuid is valid") {
      REQUIRE(test.IsValid());
    }
  }

  GIVEN("a uuid created with value FFBC5192-D8A1-EF24-0385-68C2D9EBCF15 by two uint64_ts") {
    sonic::UUID test(0xFFBC5192D8A1EF24, 0x038568C2D9EBCF15);
    THEN("uuid matches the test UUID") {
      REQUIRE(test == testUUID1);
    }
    AND_THEN("uuid is valid") {
      REQUIRE(test.IsValid());
    }
  }
}

SCENARIO("Creating a UUID from string") {
  GIVEN("\"FFBC5192-D8A1-EF24-0385-68C2D9EBCF15\" is a valid UUID string") {
    REQUIRE(sonic::UUID::IsValidUUID(testUUID1Str.c_str()));
    AND_GIVEN("a uuid created with the string FFBC5192-D8A1-EF24-0385-68C2D9EBCF15") {
      sonic::UUID test(testUUID1Str);
      THEN("uuid matches the test UUID") {
        REQUIRE(test == testUUID1);
      }
      AND_THEN("uuid is valid") {
        REQUIRE(test.IsValid());
      }
    }
  }
}

SCENARIO("Serializing a UUID to string") {
  GIVEN("a UUID created with the value FFBC5192-D8A1-EF24-0385-68C2D9EBCF15") {
    sonic::UUID test(testUUID1Str);
    AND_GIVEN("the string returned by serializing the UUID") {
      std::string uuidStr = test.ToString();
      THEN("string matches the test UUID string") {
        REQUIRE(test == testUUID1);
        REQUIRE(uuidStr == testUUID1Str);
      }
    }
  }
}

SCENARIO("Invalid UUIDs") {
  GIVEN("the UUID created with the value FFBC5192D8A1EF24038568C2D9EBCF15") {
    sonic::UUID test("FFBC5192D8A1EF24038568C2D9EBCF15");
    THEN("the UUID is invalid") {
      REQUIRE(!test.IsValid());
    }
  }

  GIVEN("the UUID created with the value \"THISISTHIRTYSIXCHARACTERSLONGTOO\"") {
    sonic::UUID test(pathological);
    THEN("the UUID is invalid") {
      REQUIRE(!test.IsValid());
    }
  }

  GIVEN("the UUID created with the value FOBC5192-D8A1-EF24-0385-68C2D9EBCF15 (uppercase o replaces one 0)") {
    std::string fakeUUID(testUUID1Str);
    fakeUUID.at(1) = 'O';
    sonic::UUID test(fakeUUID);
    THEN("the UUID is invalid") {
      REQUIRE(!test.IsValid());
    }
  }
}

SCENARIO("Clearing a UUID") {
  GIVEN("a valid UUID of value FFBC5192-D8A1-EF24-0385-68C2D9EBCF15") {
    sonic::UUID test("FFBC5192-D8A1-EF24-0385-68C2D9EBCF15");
    REQUIRE(test.IsValid());
    AND_GIVEN("the same UUID, cleared") {
      test.Clear();
      THEN("the UUID is no longer valid") {
        REQUIRE(!test.IsValid());
      }
    }
  }
}