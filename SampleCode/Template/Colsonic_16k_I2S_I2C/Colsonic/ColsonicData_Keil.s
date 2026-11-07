;/*---------------------------------------------------------------------------------------------------------*/
;/*                                                                                                         */
;/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
;/*                                                                                                         */
;/*---------------------------------------------------------------------------------------------------------*/


    AREA _CMDData, DATA, READONLY

    EXPORT  u32CMDDataBegin
    EXPORT  u32CMDDataEnd

u32CMDDataBegin
	INCBIN ..\Colsonic\colsonicdata.bin
u32CMDDataEnd

	END