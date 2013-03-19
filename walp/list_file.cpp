#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>

using namespace std;

int main (int argc, char **argv)
{
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " directory" << endl;
        return 1;
    }

    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(argv[1])) != NULL)
    {
	while ((ent = readdir(dir)) != NULL)
		cout << ent->d_name << endl;

	closedir(dir);
    }
    else
    {
	perror("");
	return 1;
    }

    return 0;
}
