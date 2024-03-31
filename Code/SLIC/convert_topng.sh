#!/bin/bash

# Définir le répertoire de travail
dir=$(pwd)

# Itérer sur tous les fichiers .pgm et .ppm
for file in *.pgm *.ppm; do
  # Convertir le fichier en .png
  convert "$file" "${file%.*}.png"

  # Afficher un message de confirmation
  echo "Fichier $file converti en ${file%.*}.png"
done

# Afficher un message de fin
echo "Tous les fichiers .pgm et .ppm ont été convertis en .png"
