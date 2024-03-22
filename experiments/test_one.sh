line_number=1
COMPRESSOR="N01"
new_line="#define COMPRESSOR_$COMPRESSOR"
files=("/data/pinanoraw/tgonzalez/pod5_nanoraw/pod5/c++/pod5_format/pgnano/pgnano.cpp" "/data/pinanoraw/tgonzalez/pod5_nanoraw/src/c++/copy.cpp")

for file_path in "${files[@]}";do
    
    	# Modify the lines in your C++ file as needed
    	sed -i "${line_number}s/.*/$new_line/" "$file_path"
    	echo "Line $line_number in $file_path replaced with: $new_line"
done

./build.sh c dirty release
echo -e "\n\n"
# echo "Descomprimir"
# build/src/c++/copy ../files/test_files/e951f24f-batch3.pod5 ../files/test_files/uncompressed.pod5 --uncompressed
# echo -e "\n\n"
if [ "$COMPRESSOR" = "VBZ" ]; then
    echo "Comprimir"
    build/src/c++/copy ../files/test_files/uncompressed.pod5 ../files/test_files/"$COMPRESSOR".pod5 --VBZ
    python src/python/pgnano/main_scripts/ont_check_pod5_files_equal.py ../files/test_files/e951f24f-batch3.pod5 ../files/test_files/"$COMPRESSOR".pod5
    echo -e "\n\n"
    # echo "Descomprimir y comprimir en un paso"
    # build/src/c++/copy ../files/test_files/e951f24f-batch3.pod5 ../files/test_files/"$COMPRESSOR".pod5 --VBZ
    # python src/python/pgnano/main_scripts/ont_check_pod5_files_equal.py ../files/test_files/e951f24f-batch3.pod5 ../files/test_files/"$COMPRESSOR".pod5
    # echo -e "\n\n"
else
    echo "Comprimir"	
    build/src/c++/copy ../files/test_files/uncompressed.pod5 ../files/test_files/"$COMPRESSOR".pod5 --pgnano
    build/src/c++/copy ../files/test_files/"$COMPRESSOR".pod5 ../files/test_files/"$COMPRESSOR"_test.pod5 --VBZ /dev/null > /dev/null 2>&1
    python src/python/pgnano/main_scripts/ont_check_pod5_files_equal.py ../files/test_files/e951f24f-batch3.pod5 ../files/test_files/"$COMPRESSOR"_test.pod5
    echo -e "\n\n"

    # echo "Descomprimir y comprimir en un paso"
    # build/src/c++/copy ../files/test_files/e951f24f-batch3.pod5 ../files/test_files/"$COMPRESSOR".pod5 --pgnano
    # build/src/c++/copy ../files/test_files/"$COMPRESSOR".pod5 ../files/test_files/"$COMPRESSOR"_test.pod5 --VBZ /dev/null > /dev/null 2>&1
    # python src/python/pgnano/main_scripts/ont_check_pod5_files_equal.py ../files/test_files/e951f24f-batch3.pod5 ../files/test_files/"$COMPRESSOR"_test.pod5
    # echo -e "\n\n"

fi
