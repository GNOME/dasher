#include "gtest/gtest.h"
#include "../../Src/TestPlatform/MockFileWordGenerator.h"
using namespace Dasher;

/*
 * Test fixture
 */
class WordGenTest : public ::testing::Test {
  public:
    WordGenTest() {
      fullGen = new MockFileWordGenerator("test_data/word_gen_full_data.txt");
      singleLineGen = new MockFileWordGenerator("test_data/word_gen_single_line.txt");
    }
  
  protected:
    MockFileWordGenerator* fullGen;
    MockFileWordGenerator* singleLineGen;
};

/*
 * Tests whether the word fullGenerator opens a file
 * correctly.
 */
TEST_F(WordGenTest, FileOpensCorrectly) {
  bool opened = fullGen->GetFileHandle().is_open();
  bool good = fullGen->GetFileHandle().good();
  ASSERT_EQ(opened, true);
  ASSERT_EQ(good, true);
}

/*
 * Tests whether the file name getter returns expected results.
 */
TEST_F(WordGenTest, FilenameExtraction) {
  ASSERT_EQ("word_gen_full_data.txt", fullGen->GetFilename());
}

/*
 * Tests if retrieving the next word returns expected results.
 */
TEST_F(WordGenTest, WordIteration) {
  ASSERT_EQ("I'm", fullGen->GetNextWord());
  ASSERT_EQ("a", fullGen->GetNextWord());
}

/*
 * Tests if the word generator actually grabs the right amount of data
 * from the stream
 */
TEST_F(WordGenTest, GetBuffer) {
  std::ifstream fin;
  std::string buffer;
  fin.open("test_data/word_gen_single_line.txt");
  std::getline(fin, buffer);
  
  ASSERT_EQ(buffer, singleLineGen->GetWholeBuffer());
}

/* This test /should/ determine whether we return the right value when
 * we reach the end of a file...Not sure what's causing this to fail now
 */
TEST_F(WordGenTest, EndsAtEOF) {
  for(int i = 0; i < 1000; i++)
    singleLineGen->GetNextWord();
    
  ASSERT_EQ("", singleLineGen->GetNextWord());
}


