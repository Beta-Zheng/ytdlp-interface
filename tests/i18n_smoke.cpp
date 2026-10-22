#include "../ytdlp-interface/i18n.hpp"
#include <cassert>
#include <iostream>
#include <string>

int main()
{
    assert(i18n::tr("Settings") == "设置");
    assert(i18n::tr("Show queue") == "显示队列");
    assert(i18n::tr("Cancel") == "取消");
    assert(i18n::tr("queued") == "已排入队列");
    assert(i18n::tr("done") == "完成");
    assert(i18n::tr("Start item #12") == "开始项目 #12");
    assert(i18n::tr("Stop item #12") == "停止项目 #12");
    assert(i18n::tr("ytdlp-interface v2.19.1 - settings") == "ytdlp-interface v2.19.1 - 设置");
    assert(i18n::trw(L"Settings") == L"设置");
    assert(i18n::trw(L"E:\\下载\\唯一😀.mp4") == L"E:\\下载\\唯一😀.mp4");
    const std::string unknown = "原始标题😀 and an unknown label";
    assert(i18n::tr(unknown) == unknown);
    assert(i18n::tr("").empty());
    std::cout << "i18n smoke checks passed\n";
}
