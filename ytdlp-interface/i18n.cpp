#include "i18n.hpp"

#include <Windows.h>

#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace
{
	std::wstring from_utf8(std::string_view text)
	{
		if(text.empty())
			return {};

		const int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
		std::wstring result(size, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), size);
		return result;
	}

	std::string to_utf8(std::wstring_view text)
	{
		if(text.empty())
			return {};

		const int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
		std::string result(size, '\0');
		WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), size, nullptr, nullptr);
		return result;
	}

	std::string translate_regex(std::string_view input)
	{
		static const std::vector<std::pair<std::regex, std::string>> rules
		{
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - settings$)"}, "$1 - 设置"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - subtitle selection$)"}, "$1 - 字幕选择"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - queue finished$)"}, "$1 - 队列完成"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - format sorting info$)"}, "$1 - 格式排序信息"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - manual selection of formats$)"}, "$1 - 手动选择格式"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - media sections$)"}, "$1 - 媒体片段"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - JSON viewer$)"}, "$1 - JSON 查看器"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - release notes$)"}, "$1 - 更新说明"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - custom dark color theme$)"}, "$1 - 自定义深色主题"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+) - custom light color theme$)"}, "$1 - 自定义浅色主题"},
			{std::regex{R"(^(ytdlp-interface v[\d.]+)\s+\((.+) is available\)$)"}, "$1（可更新为 $2）"},
			{std::regex{R"(^(.*) \(queue item #(\d+)\)$)"}, "$1（队列项目 #$2）"},
			{std::regex{R"(^The URL in the clipboard is already added \(queue item #(\d+)\)\.$)"}, "剪贴板中的网址已加入队列（项目 #$1）。"},
			{std::regex{R"(^Download options for queue item #(\d+)$)"}, "队列项目 #$1 的下载选项"},
			{std::regex{R"(^Start item #(\d+)$)"}, "开始项目 #$1"},
			{std::regex{R"(^Stop item #(\d+)$)"}, "停止项目 #$1"},
			{std::regex{R"(^Resume item #(\d+)$)"}, "继续项目 #$1"},
			{std::regex{R"(^Remove item #(\d+)$)"}, "移除项目 #$1"},
			{std::regex{R"(^Open folder of item #(\d+)$)"}, "打开项目 #$1 的文件夹"},
			{std::regex{R"(^Open file of item #(\d+)$)"}, "打开项目 #$1 的文件"},
			{std::regex{R"(^Set file name of item #(\d+)$)"}, "设置项目 #$1 的文件名"},
			{std::regex{R"(^Select subtitles \(([^)]+)\)$)"}, "选择字幕（$1）"},
			{std::regex{R"(^Select videos \(([^)]+)\)$)"}, "选择视频（$1）"},
			{std::regex{R"(^Select songs \(([^)]+)\)$)"}, "选择歌曲（$1）"},
			{std::regex{R"(^Current default: (.*)$)"}, "当前默认：$1"},
			{std::regex{R"(^stopped \((.+)\)$)"}, "已停止（$1）"},
			{std::regex{R"(^\[live event scheduled to begin in (.+)\]$)"}, "[直播活动预计于 $1 开始]"},
			{std::regex{R"(^(.+)\s+\(current = (.+)\)$)"}, "$1（当前：$2）"},
			{std::regex{R"(^(.+)\s+\(current\)\s+\[click to see changelog\]$)"}, "$1（当前使用中）[点击查看变更记录]"},
			{std::regex{R"(^(.+)\s+\(current\)\s+\[click for changelog\]$)"}, "$1（当前使用中）[点击查看变更记录]"},
			{std::regex{R"(^(.+)\s+\(current\)$)"}, "$1（当前使用中）"},
			{std::regex{R"(^(.+) lacks a 32-bit package!$)"}, "$1 缺少 32 位软件包！"},
			{std::regex{R"(^(.+) lacks a Win7 package!$)"}, "$1 缺少 Win7 软件包！"},
			{std::regex{R"re(^A preset with the name "([^"]*)" already exists\.$)re"}, "名称「$1」的预设已存在。"},
			{std::regex{R"re(^The preset "([^"]*)" already contains the current settings\.$)re"}, "预设「$1」已包含当前设置。"}
		};

		const std::string text(input);
		for(const auto &[pattern, replacement] : rules)
		{
			if(std::regex_match(text, pattern))
				return std::regex_replace(text, pattern, replacement);
		}

		return text;
	}
}

std::string i18n::tr(std::string_view text)
{
	static const std::unordered_map<std::string, std::string> exact
	{
		{"none", "无"},
		{"video", "视频"},
		{"audio", "音频"},
		{"default", "默认"},
		{"not present", "未安装"},
		{"Settings", "设置"},
		{"Formats", "格式"},
		{"Queue actions", "队列操作"},
		{"Copy selected URL(s)", "复制选择的网址"},
		{"Switch view (queue/output)", "切换查看（队列/输出）"},
		{"Set file name of queue item", "设置队列项目的文件名"},
		{"Delete queue item(s)", "删除队列项目"},
		{"Close window", "关闭窗口"},
		{"Reset window size and position", "重置窗口大小与位置"},
		{"Download folder:", "下载文件夹："},
		{"Download rate limit:", "下载速率上限："},
		{"Chapters:", "章节："},
		{"Force keyframes at cuts", "切割位置强制关键帧"},
		{"Embed subtitles", "嵌入字幕"},
		{"Embed thumbnail", "嵌入缩略图"},
		{"Convert audio to MP3", "将音频转成 MP3"},
		{"File modification time = time of writing", "文件修改时间 = 写入时间"},
		{"Custom arguments:", "自定义参数："},
		{"Download options", "下载选项"},
		{"Show output", "显示输出"},
		{"Show queue", "显示队列"},
		{"Start download", "开始下载"},
		{"Stop download", "停止下载"},
		{"Preferred resolution:", "首选分辨率："},
		{"Preferred video codec:", "首选视频编码："},
		{"Preferred audio codec:", "首选音频编码："},
		{"Preferred video container:", "首选视频容器："},
		{"Preferred audio container:", "首选音频容器："},
		{"Color theme:", "色彩主题："},
		{"Contrast:", "对比度："},
		{"Output template:", "输出模板："},
		{"Max concurrent downloads:", "最大同时下载数："},
		{"Playlist indexing:", "播放列表编号："},
		{"When browsing for the output folder, start in:", "浏览输出文件夹时的起始位置："},
		{"Load cookies from browser:", "从浏览器加载 Cookies："},
		{"Additional options:", "额外选项："},
		{"Max number of concurrent yt-dlp instances used for getting data:", "用于获取数据的 yt-dlp 最大同时运行数："},
		{"Prefer a higher framerate", "首选较高帧率"},
		{"Dark", "深色"},
		{"Light", "浅色"},
		{"System preference", "跟随系统"},
		{"Start next item on lengthy processing", "长时间处理时自动开始下一个项目"},
		{"Each queue item has its own download options", "每个队列项目使用独立下载选项"},
		{"When stopping a queue item, automatically start the next one", "停止队列项目时自动开始下一个"},
		{"When the program starts, automatically start processing the queue", "程序启动时自动处理队列"},
		{"Pad the indexed filenames with zeroes", "文件名编号补零"},
		{"Put playlists in their own folders", "播放列表使用独立文件夹"},
		{"Program folder", "程序文件夹"},
		{"Currently selected folder", "当前选择的文件夹"},
		{"Mark these categories:", "标记这些分类："},
		{"Remove these categories:", "移除这些分类："},
		{"Use this proxy:", "使用此代理："},
		{"Snap windows to screen edges", "窗口贴齐屏幕边缘"},
		{"No minimum width for the main window", "主窗口不限制最小宽度"},
		{"[YouTube] For 1080p, prefer the \"premium\" format with enhanced bitrate", "[YouTube] 1080p 时优先使用高码率 premium 格式"},
		{"[YouTube] Use the Android player client for video extraction", "[YouTube] 提取视频时使用 Android 播放器用户端"},
		{"Save queue items with \"error\" status to the settings file", "将状态为「错误」的队列项目保存到设置档"},
		{"Automatically remove completed items (with \"done\" status)", "自动移除已完成项目（状态为「完成」）"},
		{"Formats window: display file sizes with exact byte value", "格式窗口：以精确字节值显示文件大小"},
		{"When the main window is activated, automatically add the URL from clipboard", "主窗口启用时自动加入剪贴板中的网址"},
		{"Display any custom file names in the \"Media title\" column", "在「媒体标题」列显示自定义文件名"},
		{"Tell yt-dlp to download with aria2c  (--downloader aria2c)", "让 yt-dlp 使用 aria2c 下载（--downloader aria2c）"},
		{"Load cookies from file:", "从文件加载 Cookies："},
		{"Close", "关闭"},
		{"Reset to default", "恢复默认"},
		{"Options for aria2c (--downloader-args):", "aria2c 选项（--downloader-args）："},
		{"yt-dlp", "yt-dlp"},
		{"SponsorBlock", "SponsorBlock"},
		{"Queuing", "队列"},
		{"Interface", "界面"},
		{"Updater", "更新器"},
		{"Config presets", "配置预设"},
		{"About", "关于"},
		{"Nana C++ GUI library", "Nana C++ GUI 库"},
		{"JSON for Modern C++", "JSON for Modern C++"},
		{"Libraries used", "使用的库"},
		{"Keyboard shortcuts", "键盘快捷键"},
		{"Press Ctrl+V or click here to paste and add media link", "按 Ctrl+V 或点这里粘贴并加入媒体链接"},
		{"output from yt-dlp.exe appears here\n\nright-click for options\n\ndouble-click to show queue", "yt-dlp.exe 的输出会显示在这里\n\n按右键可查看选项\n\n按两下可显示队列"},
		{"queued", "已排入队列"},
		{"downloading", "下载中"},
		{"processing", "处理中"},
		{"started", "已开始"},
		{"done", "完成"},
		{"error", "错误"},
		{"skip", "略过"},
		{"stopped", "已停止"},
		{"the clipboard does not contain any text", "剪贴板中没有任何文字"},
		{"* multiple lines of text, make sure they're URLs *", "* 内容包含多行文字，请确认它们都是网址 *"},
		{"checking...", "检查中..."},
		{"failed to get from GitHub!", "无法从 GitHub 获取数据！"},
		{"unable to get from GitHub", "无法从 GitHub 获取数据"},
		{"Update", "更新"},
		{"Update deno", "更新 Deno"},
		{"Update Deno", "更新 Deno"},
		{"Update yt-dlp", "更新 yt-dlp"},
		{"Update FFmpeg", "更新 FFmpeg"},
		{"Cancel", "取消"},
		{"Unpacking archive and restarting...", "正在解压缩压缩包并重新启动..."},
		{"Unpacking archive to temporary folder...", "正在将压缩包解压缩到临时文件夹..."},
		{"Copying deno.exe to yt-dlp folder...", "正在将 deno.exe 复制到 yt-dlp 文件夹..."},
		{"Deno update complete", "Deno 更新完成"},
		{"Extracting files to temporary folder...", "正在将文件解压缩到临时文件夹..."},
		{"FFmpeg update complete", "FFmpeg 更新完成"},
		{"yt-dlp update complete", "yt-dlp 更新完成"},
		{"No place to put the ffmpeg files", "没有可放置 ffmpeg 文件的位置"},
		{"No place to put yt-dlp.exe", "没有可放置 yt-dlp.exe 的位置"},
		{"File copy error", "文件复制错误"},
		{"Custom FFmpeg folder", "自定义 FFmpeg 文件夹"},
		{"The argument has been added.", "已加入参数。"},
		{"Duplicate name", "名称重复"},
		{"Duplicate preset", "预设重复"},
		{"Blank name", "空白名称"},
		{"Are you sure you want to create a preset with a blank name?", "确定要创建空白名称的预设吗？"},
		{"Are you sure you want to make the preset name blank?", "确定要将预设名称改成空白吗？"},
		{"Choose folder...", "选择文件夹..."},
		{"Clear folder history", "清除文件夹历史"},
		{"Paste", "粘贴"},
		{"Select formats", "选择格式"},
		{"View JSON data", "查看 JSON 数据"},
		{"Refresh (reacquire data)", "刷新（重新获取数据）"},
		{"Download sections", "下载片段"},
		{"Treat as playlist", "视为播放列表"},
		{"Split playlist (add videos to queue)", "分割播放列表（将视频加入队列）"},
		{"Do not download", "不要下载"},
		{"Make downloadable", "允许下载"},
		{"Toggle download ability", "切换是否可下载"},
		{"Clear completed", "清除已完成"},
		{"Start all", "全部开始"},
		{"Stop all", "全部停止"},
		{"Remove this playlist", "移除此播放列表"},
		{"Remove all", "全部移除"},
		{"Start selected", "开始所选项目"},
		{"Stop selected", "停止所选项目"},
		{"Start/stop selected", "开始/停止所选项目"},
		{"Remove selected", "移除所选项目"},
		{"Refresh selected", "刷新所选项目"},
		{"Extra columns", "额外列"},
		{"Website column", "网站列"},
		{"Favicon", "网站图示"},
		{"Text", "文字"},
		{"When finished...", "完成后..."},
		{"Shutdown", "关机"},
		{"Hibernate", "休眠"},
		{"Sleep", "睡眠"},
		{"Exit", "退出"},
		{"Format", "格式"},
		{"Format note", "格式说明"},
		{"Ext", "扩展名"},
		{"Filesize", "文件大小"},
		{"Website", "网站"},
		{"Media title", "媒体标题"},
		{"Status", "状态"},
		{"ignore", "忽略"},
		{"embed", "嵌入"},
		{"split", "分割"},
		{"shutting down in:", "将于以下时间后关机："},
		{"attempting to shut down in:", "将尝试于以下时间后关机："},
		{"hibernating in:", "将于以下时间后休眠："},
		{"attempting to hibernate in:", "将尝试于以下时间后休眠："},
		{"sleeping in:", "将于以下时间后睡眠："},
		{"attempting to sleep in:", "将尝试于以下时间后睡眠："},
		{" KB/s", " KB/秒"},
		{" MB/s", " MB/秒"},
		{"<any>", "<任何>"},
		{"Can't parse the JSON data produced by yt-dlp! See output for details.", "无法解析 yt-dlp 产生的 JSON 数据！详情请见输出内容。"},
		{"!!! failed to restore the media title !!!", "!!! 恢复媒体标题失败 !!!"},
		{"error: playlist with zero entries!", "错误：播放列表没有任何项目！"},
		{"yt-dlp failed to get info (see output)", "yt-dlp 获取信息失败（请查看输出）"},
		{"can't get data for URL, yt-dlp.exe missing!", "无法获取此网址的数据，缺少 yt-dlp.exe！"},
		{"yt-dlp did not provide any data for this URL!", "yt-dlp 没有为此网址提供任何数据！"},
		{"[channel tab] ", "[通道标签页] "},
		{"[playlist] ", "[播放列表] "},
		{"[live] ", "[直播] "},
		{"Shutting down active yt-dlp instances", "正在关闭运行中的 yt-dlp 实例"},
		{"Please wait...", "请稍候..."},
		{"ytdlp-interface - error writing settings file", "ytdlp-interface - 写入设置档错误"},
		{"ytdlp-interface JSON error", "ytdlp-interface JSON 错误"},
		{"ytdlp-interface error", "ytdlp-interface 错误"},
		{"ytdlp-interface update error", "ytdlp-interface 更新错误"},
		{"failed to open debug_log.txt for writing", "无法打开 debug_log.txt 以进行写入"},
		{"Show release notes for:", "显示此版本的更新说明："},
		{"Latest version:", "最新版本："},
		{"Release notes", "更新说明"},
		{"Check at program startup and display any new version in the title bar", "程序启动时检查更新，并在标题列显示新版本"},
		{"Only extract ytdlp-interface.exe from the downloaded archive", "下载更新压缩包时只解出 ytdlp-interface.exe"},
		{"Path to yt-dlp:", "yt-dlp 路径："},
		{"FFmpeg folder:", "FFmpeg 文件夹："},
		{"Latest yt-dlp version:", "最新 yt-dlp 版本："},
		{"Latest FFmpeg version:", "最新 FFmpeg 版本："},
		{"Latest Deno version:", "最新 Deno 版本："},
		{"yt-dlp release channel:", "yt-dlp 发行通道："},
		{"Stable", "稳定版"},
		{"Nightly", "每日构建版"},
		{"When updating ffmpeg, also extract \"ffplay.exe\"", "更新 ffmpeg 时也一并解出「ffplay.exe」"},
		{"JavaScript runtime", "JavaScript 运行环境"},
		{"aria2c.exe found in the yt-dlp folder", "已在 yt-dlp 文件夹中找到 aria2c.exe"},
		{"aria2c.exe found in the %path% system variable", "已在系统 %path% 变量中找到 aria2c.exe"},
		{"aria2c.exe not found in the yt-dlp folder or the %path% system variable", "在 yt-dlp 文件夹或系统 %path% 变量中都找不到 aria2c.exe"},
		{"Name for new preset:", "新预设名称："},
		{"Create", "创建"},
		{"Save", "保存"},
		{"Load", "加载"},
		{"Delete", "删除"},
		{"Rename", "重命名"},
		{"<bold>Configuration presets</> are snapshots of the settings found in the first three categories (yt-dlp, SponsorBlock, Queuing), as well as the ones in the \"Download options\" group in the main window. The \"Save\" button saves the current settings to the selected preset, and the \"Load\" button loads the selected preset into the current settings.", "<bold>设置预设</> 会保存前三个分类（yt-dlp、SponsorBlock、队列）以及主窗口「下载选项」群组中的设置快照。「保存」按钮会把当前设置存到选择的预设，而「加载」按钮则会把该预设应用到当前设置。"},
		{"All", "全部"},
		{"Sponsor", "赞助内容"},
		{"Intermission/Intro Animation", "开场动画／中场过场"},
		{"Endcards/Credits (Outro)", "结尾卡／片尾名单"},
		{"Unpaid/Self Promotion", "非付费／自我宣传"},
		{"Preview/Recap", "预告／回顾"},
		{"Filler Tangent/Jokes", "填充闲聊／玩笑"},
		{"Interaction Reminder (Subscribe)", "互动提醒（订阅）"},
		{"Music: Non-Music Section", "音乐视频中的非音乐片段"},
		{"Highlight", "重点片段"},
		{"Chapter", "章节"},
		{"Hook/Greetings", "开场钩子／问候"},
		{"!!!  FFMPEG.EXE NOT FOUND IN THE PROGRAM FOLDER  !!!", "!!! 程序文件夹中找不到 FFMPEG.EXE !!!"},
		{"!!!  FFMPEG.EXE NOT FOUND IN THE SELECTED FOLDER  !!!", "!!! 在选择的文件夹中找不到 FFMPEG.EXE !!!"},
		{"!!!  YT-DLP EXECUTABLE NOT FOUND IN PROGRAM FOLDER  !!!", "!!! 程序文件夹中找不到 YT-DLP 可执行文件 !!!"},
		{"!!!  YT-DLP EXECUTABLE NOT FOUND AT ITS SELECTED LOCATION  !!!", "!!! 在选定位置找不到 YT-DLP 可执行文件 !!!"}
	};

	const auto it = exact.find(std::string(text));
	if(it != exact.end())
		return it->second;

	return translate_regex(text);
}

std::wstring i18n::trw(std::wstring_view text)
{
	return from_utf8(tr(to_utf8(text)));
}
