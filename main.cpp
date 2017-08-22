#include <gtest\gtest.h>
//#include <vld.h> // memory leaks control


int main(int argc, char ** argv)
{
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
