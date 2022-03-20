// bmp_graphics.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include<Windows.h>
#include<WinUser.h>
int ix = 0;

class bmp
{
	
#define headerlength 54
private:
	int32_t width;//像素宽度
	int32_t memwidth;//内存中一行的字节数
	int32_t height;//像素高度
	int16_t bitdepth;//位深度(24或32)
	int32_t size;//总大小
	int32_t datasize;//数据块大小
public:
	uint8_t* file=0;//文件数据
	uint8_t* data;//像素数据
	int32_t x;//操作点横坐标（右下角原点）
	int32_t y;//操作点纵坐标（右下角原点）
	int32_t memoffset;//操作点内存坐标
	uint8_t rgb[4];//
	char path[MAX_PATH];//文件路径
	//把文件newpath写入内存file(原内存未删除)
	int downloadfile(const char* newpath)
	{

		if (newpath != 0)
			strcpy(path,newpath);
		if (path == 0)
			return -1;
		FILE* fp = fopen(path, "rb+");
		uint8_t* temp = new uint8_t[headerlength];

		if (fread(temp, 1, headerlength, fp) != headerlength)
		{
			delete temp;
			return -1;
		}
		width = *(int32_t*)&temp[18];
		height = *(int32_t*)&temp[22];
		bitdepth = *(int16_t*)&temp[28];
		if (bitdepth == 24)
		{
			memwidth = width * 3 / 4;
			memwidth *= 4;
			if (memwidth < width * 3)
				memwidth+=4;
		}
		else if (bitdepth == 32)
			memwidth = width * 4;
		size = *(int32_t*)&temp[2];
		datasize = *(int32_t*)&temp[34];
		fseek(fp, 0, SEEK_END);
		delete[] temp;
		if (ftell(fp) != size)
		{
			return -2;
		}
		fseek(fp, 0, 0);
		temp = new uint8_t[size];
		int fr = fread(temp, 1, size, fp);
		if ( fr!= size)
		{
			delete[] temp;
			return -3;
		}
		else
		{
			file = temp;
			data = file + headerlength;
			fclose(fp);
			return 0;
		}
	}
	//把文件写入硬盘
	int uploadfile(const char* newpath)
	{
		if (newpath != 0)
			strcpy(path, newpath);
		if (path == 0)
			return -1;
		FILE* fp = fopen(path, "wb+");
		uint8_t def[headerlength] = {
66,		77,//固定BM 0~1
0,		0,		0,		0,//文件大小size=datasize+56 2~5
0,		0,		0,		0,//固定 6~9
54,		0,		0,		0,//固定文件开头长度 10~13
40,		0,		0,		0,//固定 14~17
0,		0,		0,		0,//宽width 18~21
0,		0,		0,		0,//高height 22~25
1,		0,//固定 26~27
24,		0,//24位图bitdepth24，固定 28~29
0,		0,		0,		0,//固定 30~33
0,		0,		0,		0,//像素数据大小datasize 34~37
0,		0,		0,		0,//固定 38~41
0,		0,		0,		0,//固定 42~45
0,		0,		0,		0,//固定 46~49
0,		0,		0,		0,//固定 50~53
	//默认文件开头
		};
		*(int32_t*)&def[2] = size;
		*(int32_t*)&def[18] = width;
		*(int32_t*)&def[22] = height;
		*(int16_t*)&def[28] = bitdepth;
		*(int32_t*)&def[34] = datasize;
		fwrite(def, 1, headerlength, fp);
		fwrite(data, 1, datasize, fp);
		return 0;
	}
	//从坐标找内存
	void pointfind(void)
	{
		if (x < width && y < height)
		{
			memoffset = y * memwidth + x * (bitdepth / 8)+headerlength;
			return;
		}
	}
	void pointread(void)
	{
		pointfind();
		memcpy(rgb, file + memoffset, bitdepth / 8);
	}
	void pointwrite(void)
	{
		pointfind();
		memcpy(file + memoffset, rgb, bitdepth / 8);
	}
	//int pixelfunc(int32_t x,int32_t y,void(*fun)(int*))
	void traversalpoints(void(*func)(uint8_t*))
	{
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				pointfind();
				memcpy(rgb, file + memoffset, bitdepth / 8);;
				func(rgb);
				memcpy(file + memoffset, rgb, bitdepth / 8);
			}
		}
	}
	void printtextfile(void)
	{
		ix = 0;
		for (int i = 0; i < headerlength; i++)
		{

			printf("%3d,", file[i]);
			i++;
			printf("%3d,", file[i]);
			i++;
			printf("%3d\t", file[i]);
			ix++;
			if (ix == 4)
			{
				ix = 0;
				printf("\n");
			}
		}
		ix = 0;
		printf("\n");
		for (int i = headerlength; i < size; i++)
		{
			
			printf("%3d,", file[i]);
			i++;
			printf("%3d,", file[i]);
			i++;
			printf("%3d\t", file[i]);			ix++;
			if (ix == 4)
			{
				ix = 0;
				printf("\n");
			}
		}

	}
};

void aver(uint8_t* p)
{
	uint32_t i;
	i = p[0] + p[1] + p[2];
	p[0] = i / 3;
	p[1] = p[0];
	p[2] = p[1];
}

int main()
{
	bmp* tu=new bmp;
	tu->downloadfile("0.bmp");
	//tu->printtextfile();
	printf("\n\n");
	tu->traversalpoints(aver);
	//tu->printtextfile();
	tu->uploadfile("1.bmp");
    std::cout << "Hello World!\n";
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
