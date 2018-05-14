The graph file about G(V,E) is a text file. There are |E|+1 lines in the files. The first line contains a number |V|, and the following lines contain 3 numbers about an edge (u,v,length) each, e.g. "10 3 1" which means it is an edge from vertex 10 to vertex 3 with length 1. We should also denote the vertices in [0, |V|-1]. For example, the graph file for this graph ( http://pages.cpsc.ucalgary.ca/~jacobs/Courses/cpsc331/W12/tutorials/Figures/directed_graph_example1.gif ) is in the example.txt file.

Take example.txt as an example, we build the complete 2-hop index in the following way by g++ compiler in Linux:

g++ io_ud.h label_ud.cpp -O3 -m64 && ./a.out example.txt

Some information for debugging is outputted to screen, e.g. example.txt.log is the screenshot when running the code on example.txt, but only the last 2 lines are useful. The line "sumCnt indexSize 11 0.000247 MB" means the total number of label for all the vertices are 11 and the index size is about 0.000247 MB. The line "tRun 0.005294" means it takes 0.005294 seconds to build the 2-hop index.

Note that we use 4GB memory to build the index by default. We can also modify one value in io_ud.h line 15 to build the index with smaller memory. For example, to use 1GB memory, line 15 in io_ud.h should be:
const long long memSize = 1LL * 1024 * 1024 * 1024 ;

After building the index, some files are generated for querying, but only thre of them are useful, which are example.txt.deg, example.txt.labelout and example.txt.labelin.

We can test the memory-based querying time in the following way:

g++ io_ud.h query_ud_im.cpp -O3 && ./a.out example.txt

We can test the disk-based querying time in the following way:

g++ io_ud.h query_ud_em.cpp -O3 && ./a.out example.txt



