#include "bplustree.h"
int main()
{
	printf("header size: %d\n", sizeof(tree_header));
	printf("node size: %d\n", sizeof(node));
	printf("internal_node size: %d\n", sizeof(internal_node));
	printf("leaf_node size: %d\n", sizeof(leaf_node));
	printf("bucket size: %d\n", sizeof(ptr_bucket));
	return 0;
}
