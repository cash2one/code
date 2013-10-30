#include "file.h"

int main()
{
    file::make_dir("Top");
    //file::write_file("Top/file1", "@Table(name = \"Topfa!@#$^&*()\")");
    //file::read_file("Top/file1");
    //file::copy_file("Top/file1", "Top/file2");
    //file::insert_word("Top/file1", "Top/file3", "2", "Top");
    //file::delete_file("Top/file3");
    file::insert_word("Top/file1","2","Top");
}

