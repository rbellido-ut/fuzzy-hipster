#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <dirent.h>

using namespace std;

int main (int argc, char **argv)
{
    if (argc != 2)
    {
	cout << "Usage: " << argv[0] << " directory" << endl;
	return EXIT_FAILURE;
    }

    DIR *dir;
    struct dirent *ent;
    vector<string> files;

    if ((dir = opendir(argv[1])) != NULL)
    {
	while ((ent = readdir(dir)) != NULL)
	    if (ent->d_name[0] != '.')
		files.push_back(ent->d_name);

	closedir(dir);
    }
    else
    {
	perror("");
	return EXIT_FAILURE;
    }

    for (vector<string>::iterator i = files.begin(); i != files.end(); i++)
    {
	cout << *i << endl;
    }

    return EXIT_SUCCESS;
}
