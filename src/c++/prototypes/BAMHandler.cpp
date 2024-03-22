#include "pod5_format/pgnano/BAM_handler.h"

int main()
{

    pgnano::BAMHandler h;
    h.open_BAM_file("","");
    h.build_query_index();
    return 0;
}