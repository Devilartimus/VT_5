//Увеличить яркость вертикальными полосами по 64 пикселя.
#include <iostream>
#include <ctime>
clock_t CPP(unsigned char* buffer, unsigned width, unsigned height)
{
	clock_t time = clock();
	short bright;
	unsigned int disp;
	bright = 50;
	for (unsigned int i = 0; i < width; i++)
	{
		if ((i / 64) % 2 == 0)
		{
			for (unsigned int j = 0; j < height; j++)
			{
				for (unsigned int k = 0; k < 3; k++)
				{
					disp = (j * width + i) * 4 + k;
					short val;
					if ((val = (short)buffer[disp] + bright) > 255)
						val = 255;
					buffer[disp] = val;
				}
			}
		}
	}
	return clock() - time;
}
clock_t Assembly(unsigned char* buffer, unsigned width, unsigned height)
{
	clock_t time = clock();
	_asm {
		//esi -> источник
		//edi -> приемник
		xor ecx, ecx;
		mov edi, buffer;
		mov esi, width;
		mov bl, 50;
		xor edx, edx;
	for_i:
		cmp ecx, width;
		jge i_end;
		mov eax, ecx;
		push ebx;
		push edx;
		mov ebx, 64;
		cdq;
		xor edx, edx;
		div ebx;
		xor edx, edx;
		cdq;
		mov ebx, 2;
		div ebx;
		cmp edx, 0;
		pop edx;
		pop ebx;
		jne j_end;
	for_j:
		cmp edx, height;
		jge j_end;
		xor bh, bh;
		mov eax, edx;
		imul eax, esi; 
		add eax, ecx;
		shl eax, 2; 
		add eax, edi;
	for_k:
		cmp bh, 3;
		jge k_end;
		push edx;
		mov dl, [eax];
		add dl, bl;
		jnc skip;
		mov dl, 254;
	skip:
		mov[eax], dl;
		inc bh;
		pop edx;
		inc eax;
		jmp for_k;
	k_end:
		inc eax;
		inc edx;
		jmp for_j;
	j_end:
		inc ecx;
		xor edx, edx;
		jmp for_i;
	i_end:
	}
	return clock() - time;
}
clock_t Vect(unsigned char* buffer, unsigned width, unsigned height) {
	unsigned __int64 temp = 0x0032323200323232;
	clock_t time = clock();
	_asm {
		movq mm1, temp; // сформировали число для изменения фактора
		mov edi, buffer;
		xor ecx, ecx;
		mov esi, width;
	for_i:
		cmp ecx, width;
		jge i_end;
		mov eax, ecx;
		push ebx;
		push edx;
		mov ebx, 64;
		cdq;
		xor edx, edx;
		div ebx;
		xor edx, edx;
		cdq;
		mov ebx, 2;
		div ebx;
		cmp edx, 0;
		pop edx;
		pop ebx;
		jne j_end;
	for_j:
		mov eax, edx;
		imul eax, esi;
		add eax, ecx;
		shl eax, 2;
		add eax, edi;
		cmp edx, height;
		jge j_end;
		movq mm2, [eax];
		/*Команда PADDUSB производит сложение беззнаковых байтов.
		Если значение результата больше или меньше границ диапазона
		беззнакового байта,
		то результат операции насыщается соответственно до FFh или до 00h.*/
		paddusb mm2, mm1;
		movq[eax], mm2;
		inc edx;
		jmp for_j;
	j_end:
		add ecx, 2;
		xor edx, edx;
		jmp for_i;
	i_end:
		emms;
	}
	return clock() - time;
}
void Open(FILE*& f, const char* name, const char* mode) {
	fopen_s(&f, name, mode);
	if (f == NULL) {
		printf("Error: Can't open %s with mode %s\n", name, mode);
		exit(-2);
	}
}
unsigned char* Read(const char* name, unsigned char* header, unsigned int& height,
	unsigned int& width)
{
	FILE* in;
	unsigned char* buffer;
	unsigned int len;
	Open(in, name, "rb");
	fread(header, 1, 54, in);
	width = *(unsigned int*)(header + 18);
	height = *(unsigned int*)(header + 22);
	len = width * height * 4;
	buffer = (unsigned char*)malloc(len);
	fread(buffer, 1, len, in);
	fclose(in);
	printf("width = %d\nheight = %d\n", width, height);
	return buffer;
}
void Save(const char* name, unsigned char* header, unsigned char* buffer, unsigned int len)
{
	FILE* out;
	Open(out, name, "wb");
	fwrite(header, 1, 54, out);
	fwrite(buffer, 1, len, out);
	fclose(out);
}
int main()
{
	clock_t t1 = 0, t2 = 0, t3 = 0;
	double time1, time2, time3;
	int times = 1000;
	unsigned char* orig_pic, * buffer;
	unsigned int width, height, len;
	unsigned char head[54];
	orig_pic = Read("E:/3 term/VT/lab5/Dodge.bmp", head, height, width);
	len = width * height * 4;
	buffer = (unsigned char*)malloc(len);
	for (int i = 0; i < times; i++)
	{
		memcpy(buffer, orig_pic, len);
		t1 += CPP(buffer, width, height);
		memcpy(buffer, orig_pic, len);
		t2 += Assembly(buffer, width, height);
		memcpy(buffer, orig_pic, len);
		t3 += Vect(buffer, width, height);
	}
	time1 = ((double)t1 / CLOCKS_PER_SEC) / times;
	time2 = ((double)t2 / CLOCKS_PER_SEC) / times;
	time3 = ((double)t3 / CLOCKS_PER_SEC) / times;
	printf("\nAverage execution time per %d times:\n\n\tCPP:%lf sec \n\tAssembly:%lf sec \n\tVect:%lf sec\n\n", times, time1, time2, time3);
	memcpy(buffer, orig_pic, len);
	CPP(buffer, width, height);
	Save("outCPP.bmp", head, buffer, len);
	memcpy(buffer, orig_pic, len);
	Assembly(buffer, width, height);
	Save("outAssembly.bmp", head, buffer, len);
	memcpy(buffer, orig_pic, len);
	Vect(buffer, width, height);
	Save("outVector.bmp", head, buffer, len);
	free(buffer);
	free(orig_pic);
	system("pause");
	return 0;
}
