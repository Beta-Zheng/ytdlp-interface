# 简体中文构建说明

本项目基于 ErrorFlynn/ytdlp-interface v2.19.1，是本地简体中文修改版，并非上游官方发行包。

- 上游：https://github.com/ErrorFlynn/ytdlp-interface
- 翻译模块和部分控件适配参考：https://github.com/Terence0816/ytdlp-interface-zh-TW
- 繁体翻译及适配由 Terence0816 维护；本项目将其转为简体并补充本地化。
- 原项目及翻译参考使用 MIT 许可证；原作者版权声明见 LICENSE。

界面、设置、菜单及常用提示使用简体中文。yt-dlp/FFmpeg 原始日志、网页媒体标题、编码名称、JSON 原始数据和上游更新记录保留原文。

## 使用

退出原程序，备份 ytdlp-interface.exe 和 ytdlp-interface.json，然后用 编译得到的 ytdlp-interface.exe 替换原目录中的同名文件。
保留现有 yt-dlp.exe、deno.exe、ffmpeg.exe、ffprobe.exe、7z.dll 和配置文件。

也可以先把原安装目录复制到一个新的文件夹，再覆盖其中的界面程序试用。

注意：“更新器”中的界面程序更新仍指向官方上游，更新 ytdlp-interface 本身将恢复官方英文版。单独更新 yt-dlp、FFmpeg、Deno 不会替换中文界面。

## 从源码构建

本分支只包含简体中文界面与翻译测试，不包含运行时 Bug 修复。使用上游构建方式编译，翻译测试项目为 `tests/i18n_smoke.vcxproj`。

独立的 `build/windows-vs2022` 分支提供 VS2022 兼容调整和 `build_windows.ps1`；合并后运行该脚本，程序输出到 `outputs/windows`，并自动执行翻译测试。
