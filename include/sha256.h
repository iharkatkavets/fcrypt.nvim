/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	Header file of the sha256.c source file
	Author: Vitor Henrique Andrade Helfensteller Straggiotti Silva
	Date: 26/06/2021 (DD/MM/YYYY)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SHA256_H
#define SHA256_H

#include <stdlib.h>

/******************************************************************************/
//Return 32 bytes digest of Data array on success. Return NULL if fail.
uint8_t *sha256_data(uint8_t *Data, uint64_t DataSizeByte);

//Return 32 bytes digest of file on success. Return NULL if fail.
uint8_t *sha256_file(const char *Filename);


#endif
