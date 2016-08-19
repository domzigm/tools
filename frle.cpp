/*

Copyright (c) 2016, domzigm
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


This tool was created to unpack recovery executables for AVM devices after
they've added run length encoding to reduce the file size

*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define DBGLOG printf
#define OFFSET 0x42000u

void unpack(char* recovery, char* dump, uint32_t offset)
{
	FILE* fin;
	FILE* fout;

	char magic[] = "sqsh";
	uint8_t sqsh = 0u;
	uint8_t buffer = 0u;
	uint8_t l8 = 0u;
	uint16_t l16 = 0u;

	if (0u == fopen_s(&fin, recovery, "rb") &&
		0u == fopen_s(&fout, dump, "wb"))
	{
		fseek(fin, offset, SEEK_CUR);

		while (sqsh < 4)
		{
			fread_s(&buffer, 1u, sizeof(uint8_t), 1u, fin);
			if (magic[sqsh] == buffer)
			{
				sqsh++;
			}
			else
			{
				sqsh = 0;
			}
			if (feof(fin))
			{
				return;
			}
		}
		fwrite("sqsh", 1u, 4u, fout);

		while (!feof(fin))
		{
			fread_s(&buffer, 1u, sizeof(uint8_t), 1u, fin);

			if (buffer == 0x00u)
			{
				fread_s(&l8, 1u, sizeof(uint8_t), 1u, fin);
				DBGLOG("Writing %d times (%02x) 00\n", l8, buffer);
				if (buffer != 0u && buffer != 0xffu)
				{
					DBGLOG("Pos: %i\n", ftell(fin));
				}
				if (l8 == 0u)
				{
					break;
				}
				while (l8--)
				{
					fwrite(&buffer, 1u, 1u, fout);
				}
				continue;
			}
			else if (buffer == 0x80u)
			{
				fread_s(&l8, 1u, sizeof(uint8_t), 1u, fin);
				DBGLOG("Fill %d @ %08x\n", l8, ftell(fin));
				fread_s(&buffer, 1u, sizeof(uint8_t), 1u, fin);
				while (l8--)
				{
					fwrite(&buffer, 1u, 1u, fout);
				}
				continue;
			}
			else if (buffer == 0x81u)
			{
				fread_s(&l16, sizeof(l16), sizeof(uint16_t), 1u, fin);
				fread_s(&buffer, 1u, sizeof(uint8_t), 1u, fin);
				DBGLOG("Writing %d times (%02x) 00\n", l16, buffer);
				if (buffer != 0u && buffer != 0xffu)
				{
					DBGLOG("Pos: %i\n", ftell(fin));
				}
				while (l16--)
				{
					fwrite(&buffer, 1u, 1u, fout);
				}
				continue;
			}
			else if (buffer == 0x82u)
			{
				buffer = ' ';
				fread_s(&l8, 1u, sizeof(uint8_t), 1u, fin);
				while (l8--)
				{
					fwrite(&buffer, 1u, 1u, fout);
				}
				continue;
			}
			else if (buffer < 0x80u)
			{
				l8 = buffer;
				DBGLOG("Copy %d @ %08x\n", l8, ftell(fin));
				while (l8--)
				{
					fread_s(&buffer, 1u, sizeof(uint8_t), 1u, fin);
					fwrite(&buffer, 1u, 1u, fout);
				}
				continue;
			}
			else if (buffer > 0x80u)
			{
				l8 = buffer - 0x80u;
				DBGLOG("Fill %d @ %08x\n", l8, ftell(fin));
				fread_s(&buffer, 1u, sizeof(uint8_t), 1u, fin);
				while (l8--)
				{
					fwrite(&buffer, 1u, 1u, fout);
				}
				continue;
			}

			fflush(fout);
		}

		fclose(fin);
	}
}

int main(int argc, char** argv)
{
	uint32_t offset = OFFSET;

	if (argc != 3 || argc != 4) {
		printf("Usage: %s <recovery.exe> <output.ext> [data offset = 0x42000]\n", argv[0]);
	}

	if (argc == 4) {
		offset = strtoul(argv[3], 0, 16);
	}

	unpack(argv[1], argv[2], offset);

	return 0;
}
