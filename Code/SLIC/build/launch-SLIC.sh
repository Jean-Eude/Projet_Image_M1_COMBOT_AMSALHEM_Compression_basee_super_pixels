#!/bin/sh
bindir=$(pwd)
cd /home/evan/Bureau/M1/Sem2/Projet Image/Projet_Image_M1_COMBOT_AMSALHEM_Compression_basee_super_pixels/Code/SLIC/SLIC/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /home/evan/Bureau/M1/Sem2/Projet\ Image/Projet_Image_M1_COMBOT_AMSALHEM_Compression_basee_super_pixels/Code/SLIC/build/SLIC 
	else
		"/home/evan/Bureau/M1/Sem2/Projet\ Image/Projet_Image_M1_COMBOT_AMSALHEM_Compression_basee_super_pixels/Code/SLIC/build/SLIC"  
	fi
else
	"/home/evan/Bureau/M1/Sem2/Projet\ Image/Projet_Image_M1_COMBOT_AMSALHEM_Compression_basee_super_pixels/Code/SLIC/build/SLIC"  
fi
