#include "../gui.hpp"
#include <regex>


void GUI::fm_sections()
{
	using widgets::theme;
	using std::string;
	using namespace nana;

	auto &bottom {bottoms.current()};
	themed_form fm {nullptr, *this, {}, appear::decorate<appear::minimize>{}};
	fm.caption(title + " - media sections");
	fm.snap(conf.cbsnap);
	fm.center(788, 678);
	fm.div(R"(vert margin=[15,20,20,20] 
		<weight=115 <l_help>> <weight=20> 
		<weight=25
			<l_start weight=142> <weight=10> <tbfirst weight=80> <weight=10> <l_end weight=16> <weight=10> <tbsecond weight=80>
			<weight=20> <btnadd weight=100> <weight=20> <btnremove weight=150> <weight=20> <btnclear weight=90>
		>
		<weight=20> <lbs> <weight=20> 
		<weight=35 <> <btnclose weight=100> <>>
	)");

	class tbox : public ::widgets::Textbox
	{
	public:
		tbox(window parent) : Textbox(parent)
		{
			multi_lines(false);
			text_align(align::center);
			typeface(paint::font_info {"Segoe UI", 11});
			caption("0");
			set_accept([this](char c)
			{
				if(c == keyboard::paste)
				{
					std::string cliptext {to_utf8(util::get_clipboard_text())}, pasted;
					for(auto c : cliptext)
						if(isdigit(c) || c == ':')
							pasted += c;
					if(!pasted.empty())
					{
						util::set_clipboard_text(0, to_wstring(pasted.substr(0, 9)));
						select(true);
						paste();
					}
					return false;
				}
				const auto selpoints {selection()};
				const auto selcount {selpoints.second.x - selpoints.first.x};
				return c == keyboard::backspace || c == keyboard::del || c == keyboard::undo || c == keyboard::redo || c == keyboard::cut ||
					(text().size() < 9 && (isdigit(c) || c == ':'));
			});
			events().focus([this](const arg_focus &arg)
			{
				if(arg.getting)
					select(true);
				else select(false);
			});
		}
	};

	::widgets::Text l_start {fm, "片段开始时间"}, l_end {fm, "至"};
	::widgets::Label l_help {fm, ""};
	::widgets::Button btnadd {fm, "添加到列表", true}, btnremove {fm, "从列表移除", true}, btnclear {fm, "清空列表", true},
		btnclose {fm, "Close"};
	::widgets::Listbox lbs {fm, nullptr, true};
	tbox tbfirst {fm}, tbsecond {fm};

	fm["l_help"] << l_help;
	fm["l_start"] << l_start;
	fm["l_end"] << l_end;
	fm["tbfirst"] << tbfirst;
	fm["tbsecond"] << tbsecond;
	fm["lbs"] << lbs;
	fm["btnclose"] << btnclose;
	fm["btnadd"] << btnadd;
	fm["btnremove"] << btnremove;
	fm["btnclear"] << btnclear;

	lbs.sortable(false);
	lbs.show_header(false);
	lbs.enable_single(true, false);
	lbs.typeface(paint::font_info {"Calibri", 12});
	lbs.scheme().item_height_ex = 8;
	tbfirst.focus();

	l_help.typeface(paint::font_info {"Tahoma", 10});
	l_help.text_align(align::left, align_v::top);
	l_help.format(true);
	std::string helptext {"在此设置仅下载媒体的一个或多个片段。输入开始时间和结束时间，然后添加到列表。\n\n时间格式：<bold color=0x>[小时:][分钟:]秒</>。例如，54 表示第 54 秒，9:54 表示 9 分 54 秒，1:9:54 表示 1 小时 9 分 54 秒。“至”设为 0 或留空表示媒体结尾。\n\n片段下载通过 FFmpeg 完成，速度可能较慢；每个片段保存为独立文件。"};

	for(auto &val : bottom.sections)
	{
		const auto text {to_utf8(val.first.empty() ? L"0" : val.first) + " -> " +
			(val.second.empty() || val.second == L"0" ? "end" : to_utf8(val.second))};
		lbs.at(0).push_back(text);
		lbs.at(0).back().value(val);
	}

	lbs.events().selected([&](const arg_listbox &arg)
	{
		if(arg.item.selected())
		{
			try
			{
				auto val {arg.item.value<std::pair<std::wstring, std::wstring>>()};
				tbfirst.caption(val.first);
				tbsecond.caption(val.second);
			}
			catch(...) {}
		}
	});

	if(lbs.at(0).size())
		lbs.at(0).at(0).select(true);

	tbsecond.events().key_press([&](const arg_keyboard &arg)
	{
		if(arg.key == keyboard::enter)
			btnadd.events().click.emit({}, btnadd);
	});

	tbfirst.events().key_press([&](const arg_keyboard &arg)
	{
		if(arg.key == keyboard::enter)
			tbsecond.focus();
	});

	btnclose.events().click([&] { fm.close(); });
	btnclear.events().click([&] { lbs.clear(); tbfirst.caption(""); tbsecond.caption(""); });

	btnremove.events().click([&]
	{
		auto sel {lbs.selected()};
		if(!sel.empty())
			lbs.erase(lbs.at(sel.front()));
		tbfirst.caption("");
		tbsecond.caption("");
		tbfirst.focus();
	});

	btnadd.events().click([&]
	{
		using namespace std;

		struct timepoint
		{
			timepoint(string str)
			{
				vector<int> vals, hms {0, 0, 0};
				stringstream ss {str};
				for(string val; getline(ss, val, ':');)
					vals.push_back(stoi(val));
				for(auto it_src {vals.rbegin()}, it_dest {hms.rbegin()}; it_src != vals.rend(); it_src++, it_dest++)
					*it_dest = *it_src;
				value = chrono::hours {hms[0]} + chrono::minutes{hms[1]} + chrono::seconds{hms[2]};
			}
			bool operator < (const timepoint &o) { return value < o.value; }
			bool operator > (const timepoint &o) { return value > o.value; }
			//bool operator == (const timepoint &o) { return  value == o.value; }
			bool equal(const timepoint &o) { return value == o.value; }
		private:
			chrono::seconds value;
		};

		auto first {tbfirst.text()}, second {tbsecond.text()};
		if(first.empty()) first = '0';
		if(second.empty()) second = '0';
		timepoint tp1 {first}, tp2 {second};
		if(first == "0" && second == "0")
		{
			(::widgets::msgbox {fm, "输入错误"}.icon(nana::msgbox::icon_error) << "该片段覆盖整个媒体，无需单独设置。")();
			return;
		}
		if(tp1.equal(tp2))
		{
			(::widgets::msgbox {fm, "输入错误"}.icon(nana::msgbox::icon_error) << "开始时间和结束时间相同。")();
			return;
		}
		else if(tp1 > tp2 && second != "0")
		{
			(::widgets::msgbox {fm, "输入错误"}.icon(nana::msgbox::icon_error) << "结束时间不能早于开始时间。")();
			return;
		}


		const auto text {first + " -> " + (second == "0" ? "end" : second)};
		for(auto item : lbs.at(0))
			if(item.text(0) == text)
			{
				item.select(true);
				return;
			}
		lbs.at(0).append(text).select(true);
		lbs.at(0).back().value(make_pair<wstring, wstring>(to_wstring(first), to_wstring(second)));
		tbfirst.focus();
	});

	fm.events().unload([&]
	{
		bottom.sections.clear();
		for(auto item : lbs.at(0))
			bottom.sections.push_back(item.value<std::pair<std::wstring, std::wstring>>());
	});

	fm.theme_callback([&](bool dark)
	{
		apply_theme(dark);
		fm.bgcolor(theme::fmbg);
		l_help.caption(std::regex_replace(helptext, std::regex {"\\b(0x)"}, theme::is_dark() ? "0xf5c040" : "0x4C4C9C"));
		return false;
	});

	if(conf.cbtheme == 2)
		fm.system_theme(true);
	else fm.dark_theme(conf.cbtheme == 0);

	fm.collocate();
	lbs.append_header("", lbs.size().width - fm.dpi_scale(25));
	fm.modality();
}