#include "ccformat.h"
#include "ccscan.h"

int main()
{
    chainer::cscan<size_t> t; // 假定为64位 32位改uint32_t

    memtool::base::target_pid = memtool::base::get_pid("gg.pointers");
    if (memtool::base::target_pid == -1)
        return -1;
    
    // memtool::base::target_pid = 9999; 获取有错就先这样
    
    printf("pid %d\n", memtool::base::target_pid);

    memtool::extend::get_target_mem();

    memtool::extend::set_mem_ranges(memtool::Anonymous + memtool::C_alloc + memtool::C_bss + memtool::C_data);

    printf("%d\n", t.get_pointers(0, 0, false, 10, 1 << 20));

    std::vector<size_t> addr;
    addr.emplace_back(0x715334ED44);

    auto f = fopen("1", "wb+");
    printf("%ld\n", t.scan_pointer_chain(addr, 10, 500, false, 0, f)); // 10层 偏移500
    fclose(f); // 现在已经结束了 后面的是格式化

    /*chainer::cformat<size_t> t2;
    auto f2 = fopen("1", "rb+");

    printf("%ld\n", t2.format_bin_chain_data(f2, "2", 0)); // 文件
    //  printf("%ld\n", t2.format_bin_chain_data(f2, "2", 1)); // 文件夹 需要在当前目录有2文件夹

    fclose(f2);*/
    return 0;
}
