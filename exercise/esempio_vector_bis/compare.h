#pragma once

int compare(const void* a, const void* b) {

	int aa = *(int*)a;
	int bb = *(int*)b;

	if (aa > bb)
	{
		return 1;
	}
	if (aa < bb)
	{
		return -1;
	}
	return 0;
}
