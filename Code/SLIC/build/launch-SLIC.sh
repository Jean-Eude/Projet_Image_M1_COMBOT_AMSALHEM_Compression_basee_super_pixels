#!/bin/sh
bindir=$(pwd)
cd /home/orenu/Documents/Cours/TP_CCI/ProjetCompressionImage/projet_SuperPixels/Projet_Image_M1_COMBOT_AMSALHEM_Compression_basee_super_pixels/Code/SLIC/SLIC/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /home/orenu/Documents/Cours/TP_CCI/ProjetCompressionImage/projet_SuperPixels/Projet_Image_M1_COMBOT_AMSALHEM_Compression_basee_super_pixels/Code/SLIC/build/SLIC 
	else
		"/home/orenu/Documents/Cours/TP_CCI/ProjetCompressionImage/projet_SuperPixels/Projet_Image_M1_COMBOT_AMSALHEM_Compression_basee_super_pixels/Code/SLIC/build/SLIC"  
	fi
else
	"/home/orenu/Documents/Cours/TP_CCI/ProjetCompressionImage/projet_SuperPixels/Projet_Image_M1_COMBOT_AMSALHEM_Compression_basee_super_pixels/Code/SLIC/build/SLIC"  
fi
