aws s3 cp --no-sign-request "s3://ont-open-data/contrib/melanogaster_bkim_2023.01/flowcells/D.melanogaster.R1041.400bps/D_melanogaster_1/20221217_1251_MN20261_FAV70669_117da01a/fast5/FAV70669_117da01a_45f6321d_55.fast5" ./

eval "$(conda shell.bash hook)"
conda activate nanoRawEnv

pod5 convert fast5 FAV70669_117da01a_45f6321d_55.fast5 --output FAV70669_117da01a_45f6321d_55.pod5

eval "$(conda shell.bash hook)"
conda deactivate

rm FAV70669_117da01a_45f6321d_55.fast5


