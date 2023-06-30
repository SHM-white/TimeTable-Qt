#include "include.h"
#include "Timetable.h"
#include "include\json\json.h"
#include "CSVEditor.h"
#include "Lesson.h"


//TimeTable类的实现函数
//添加课程的4参数重载函数，兼容旧代码（其实就是懒），默认写入成员变量保存的路径
int TimeTable::mAddLesson(const std::string& week, const std::string& Lesson, const std::string& sBegin, const std::string& sEnd)
{
	return mAddLesson(week, Lesson, sBegin, sEnd, mLessonInfoPath);
}
//添加课程5参数函数
int TimeTable::mAddLesson(const std::string& week, const std::string& Lesson, const std::string& sBegin, const std::string& sEnd, const std::string& TargetFilePath)
{
	if (!((bool)week.size() && (bool)Lesson.size() && (bool)sBegin.size() && (bool)sEnd.size() && (bool)TargetFilePath.size())) {
		return 0;
	}
	Json::Value root = GetRootJsonValue(TargetFilePath);
	Json::Value Current;
	Current.append(Lesson);
	Current.append(sBegin);
	Current.append(sEnd);
	root[week]["Lessons"].append(Current);
	return SaveJson(TargetFilePath,root);
}
int TimeTable::mAddLesson(const Lesson& lesson)
{
	return mAddLesson(lesson,mLessonInfoPath);
}
int TimeTable::mAddLesson(const Lesson& lesson, const std::string& TargetFilePath)
{
	return mAddLesson(lesson.mGetDay(), lesson.mGetName(),lesson.mGetBeginTimeAsString(), lesson.mGetEndTimeAsString(), TargetFilePath);
}
//添加更多信息，有空也加个重载
int TimeTable::mAddMoreInfo(const std::string& Days, const std::string& Info)
{
	std::fstream os;
	os.open(mLessonInfoPath, std::ios::out | std::ios::in);
	if (!os.is_open()) {
		return 0;
	}
	Json::Reader reader;
	Json::Value root;
	Json::StyledWriter sw;
	reader.parse(os, root);
	root[Days]["Infos"].append(Info);
	os.seekp(std::ios::beg);
	os << sw.write(root);
	os.close();
	os.clear();
	return 0;
}
int TimeTable::mReloadLesson()
{
	CurrentLesson = Lesson();
	return 0;
}
//转换时间为分钟数便于比较大小
int TimeTable::mHHMMToMin(int input)
{
	return (input - input % 100) / 100 * 60 + input % 100;
}
//暂时替换文件路径，同时返回旧的文件路径
std::string TimeTable::mReplacePath(const std::string& Path)
{
	std::string old = mLessonInfoPath;
	mLessonInfoPath = Path;
	return old;
}
Lesson TimeTable::mGetLesson(const std::string& week, int index)
{
	Json::Reader reader;
	Json::Value root;
	std::ifstream in(mLessonInfoPath, std::ios::in);
	if ((!in.is_open())||(!reader.parse(in, root)))
	{
		return Lesson();
	};
	in.close();
	const Json::Value Lessons = root[week]["Lessons"][index];
	return 	Lesson(week, Lessons[0].asString(), atoi(Lessons[1].asString().c_str()), atoi(Lessons[2].asString().c_str()));
}
Json::Value TimeTable::GetRootJsonValue(const std::string& TargetPath)
{
	Json::Reader reader;
	Json::Value root;
	std::ifstream in(TargetPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	reader.parse(in, root);
	return root;
}
//获取所有的课程并返回至传入的数组
int TimeTable::mGetLesson(std::vector<std::string>& input)
{
	input.clear();
	std::string Days[]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	for (const auto& a : Days) {
		mGetLesson(input, a);
	}
	return 1;
}
//获取今日的信息并存入传入的数组
int TimeTable::mGetTodayMoreInfo(std::vector<std::string>& input)
{
	std::string week{ mGetCurrentTime("%a") };
	return mGetTodayMoreInfo(input, week);
}
int TimeTable::mGetTodayMoreInfo(std::vector<std::string>& input, const std::string week)
{
	std::ifstream in(mLessonInfoPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(in, root)) {
		const Json::Value Infos = root[week]["Infos"];
		for (unsigned int i = 0; i < Infos.size(); ++i) {
			input.push_back(Infos[i].asString());
		}
	}
	in.close();
	return 1;
}
//获取当前时间的课程
std::string TimeTable::mGetCurrentLesson(const std::string& LessonNull)
{
	int lessonIndex = mGetCurrentLesson(0);
	return CurrentLesson.mGetName();
}

std::string TimeTable::mGetCurrentTime(const std::string& TextFormat)
{
	char result[100];
	tm structm;

	mGetCurrentTime(structm);
	auto location = TextFormat.find('%',0);
	if (location >= TextFormat.size()) {
		location = 0;
	}
	if (TextFormat.size()>=2 && !std::regex_match(TextFormat.begin() + location, TextFormat.begin() + location + 2, std::regex("%[^aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%]|%[0-9]{1,}[a-zA-Z]"))) {
		strftime(result, sizeof(result), TextFormat.c_str(), &structm);
	}
	else
	{
		return TextFormat;
	}
	return std::string(result);
}
//从csv导入课程至指定文件
int TimeTable::mImportLessonsFromCsv(const std::string& path, const std::string& TargetFileName)
{
	CSVEditor CsvEditor{ path };
	int a{ 0 };
	if (CsvEditor.mGetCsvData()) {
		for (int i{ 1 }; i < CsvEditor.mGetLineCount(); i++) {//jump line 1
			a = mAddLesson(CsvEditor[i][0], CsvEditor[i][1], CsvEditor[i][2], CsvEditor[i][3], TargetFileName);
		}
		return a;
	}
	return 0;
}
int TimeTable::sortLessons(const std::string& Day)
{
	return sortLessons(mLessonInfoPath,Day);
}
//排序
int TimeTable::sortLessons(const std::string& lessonPath, const std::string& Day)
{
	std::ifstream in(lessonPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(in, root)) { 
		in.close();
		return 0; 
	}
	std::fstream os(lessonPath, std::ios::out | std::ios::trunc);
	if (!os.is_open()) { return 0; }
	
	std::vector<Lesson> vectorLessons;

	for (const auto& i : Days) {
		if ((Day != i) && (Day != "All")) { continue; }
		if (i == "Null") { continue; }
		Json::Value& valueLessons = root[i]["Lessons"];
		vectorLessons.clear();
		for (const auto& value : valueLessons) {
			vectorLessons.push_back(
				Lesson(i,
					value[0].asString(),
					atoi(value[1].asString().c_str()),
					atoi(value[2].asString().c_str())
				)
			);
		}
		std::sort(vectorLessons.begin(), vectorLessons.end(), [](const Lesson& lesson1, const Lesson& lesson2) {return lesson1.mGetBeginTime() < lesson2.mGetBeginTime(); });
		valueLessons.clear();
		valueLessons = Json::Value(Json::arrayValue);
		for (const auto& lesson : vectorLessons) {
			Json::Value a = lesson.GetJsonValue();
			valueLessons.append(a);
		}
	}
	

	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	return 1;
}

//int TimeTable::insert(size_t index, const Lesson& lesson, const std::string& targetFile)
//{
//	return 0;
//}
//获取课程信息地址
const std::string& TimeTable::mGetLessonInfoPath()
{
	return mLessonInfoPath;
}
int TimeTable::mMinToHHMM(int input)
{
	int Hour = (input - input % 60) / 60;
	return Hour*100+input%60;
}
//获取当前时间并存入传入的结构中
int TimeTable::mGetCurrentTime(tm& tmTime)
{
	time_t timep;
	time(&timep);
	localtime_s(&tmTime, &timep);
	return 0;
}
//暂未实现
//int TimeTable::insert(size_t index, const Lesson& lesson)
//{
//	return insert(index,lesson,mLessonInfoPath);
//}
//删除课程
int TimeTable::deleteLesson(size_t index, const std::string& day, const std::string& lessonPath)
{
	std::ifstream in(lessonPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(in, root)) {
		in.close();
		return 0;
	}
	std::fstream os(lessonPath, std::ios::out | std::ios::trunc);
	if (!os.is_open()) { return 0; }

	Json::Value& valueLessons = root[day]["Lessons"];
	std::vector<Lesson> vectorLessons;
	vectorLessons.clear();
	for (const auto& value : valueLessons) {
		vectorLessons.push_back(
			Lesson(day,
				value[0].asString(),
				atoi(value[1].asString().c_str()),
				atoi(value[2].asString().c_str())
			)
		);
	}

	vectorLessons.erase(vectorLessons.begin() + index);

	//write back to json value
	valueLessons.clear();
	valueLessons = Json::Value(Json::arrayValue);
	for (const auto& lesson : vectorLessons) {
		valueLessons.append(lesson.GetJsonValue());
	}
	

	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	mReloadLesson();
	return 1;
}
int TimeTable::deleteInfo(size_t index, const std::string& day)
{
	return deleteInfo(index, day, mLessonInfoPath);
}
int TimeTable::deleteInfo(size_t index, const std::string& day, const std::string& lessonPath)
{
	Json::Value root = GetRootJsonValue(lessonPath);
	std::vector<std::string> vectorInfos;
	Json::Value& valueInfos = root[day]["Infos"];
	for (const auto& a : valueInfos) {
		vectorInfos.push_back(a.asString());
	}
	vectorInfos.erase(vectorInfos.begin()+index);
	valueInfos.clear();
	for (const auto& a : vectorInfos) {
		valueInfos.append(a);
	}
	return SaveJson(lessonPath,root);
}
int TimeTable::changeLesson(size_t index, const std::string& day, const Lesson& lesson)
{
	return changeLesson(index,day,lesson,mLessonInfoPath);
}
int TimeTable::changeLesson(size_t index, const std::string& day, const Lesson& lesson, const std::string& lessonPath)
{
	std::ifstream in(lessonPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(in, root)) {
		in.close();
		return 0;
	}
	std::fstream os(lessonPath, std::ios::out | std::ios::trunc);
	if (!os.is_open()) { return 0; }
	
	Json::Value& valueLessons = root[day]["Lessons"];
	valueLessons[(int)index] = lesson.GetJsonValue();

	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	mReloadLesson();
	return 1;
}
int TimeTable::SaveJson(const std::string& TargetPath, const Json::Value& root)
{
	std::fstream os(TargetPath, std::ios::out | std::ios::trunc);
	if (!os.is_open()) { return 0; }

	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	mReloadLesson();
	return 1;
}
int TimeTable::mGetCurrentLesson(int)
{
	static int currentIndex{ 0 };
	std::string timeCurrentTime{ mGetCurrentTime("%H%M") };
	int iCurrentTime = mHHMMToMin(atoi(timeCurrentTime.c_str()));
	if (!((CurrentLesson.mGetBeginTime() <= iCurrentTime) && (iCurrentTime <= CurrentLesson.mGetEndTime()))) {
		Json::Value root = GetRootJsonValue(mLessonInfoPath);
		std::string week{ mGetCurrentTime("%a") };
		const Json::Value Lessons = root[week]["Lessons"];
		for (unsigned int i = 0; i < Lessons.size(); ++i) {
			std::string sBeginTime = Lessons[i][1].asString();
			std::string sEndTime = Lessons[i][2].asString();
			int iBeginTime = mHHMMToMin(atoi(sBeginTime.c_str()));
			int iEndTime = mHHMMToMin(atoi(sEndTime.c_str()));
			if ((iBeginTime <= iCurrentTime) && (iEndTime >= iCurrentTime)) {
				CurrentLesson.mSetValue(Lessons[i][0].asString(), iBeginTime, iEndTime);
				currentIndex = i;
				break;
			}
		}
	}
	return currentIndex;
}
int TimeTable::deleteLesson(size_t index, const std::string& day)
{
	return deleteLesson(index,day,mLessonInfoPath);
}
//获取当前时间至指定时间的倒计时
std::string TimeTable::mGetCountDown(tm tmIn, const std::string& TimeFormat)
{
	time_t timeIn{ mktime(&tmIn) };
	time_t timeCurrent;
	time(&timeCurrent);
	timeIn -= timeCurrent;
	if (timeIn < 0) {
		timeIn = 0;
	}
	gmtime_s(&tmIn, &timeIn);
	char tmp[256];
	snprintf(tmp, sizeof(tmp) - 1, TimeFormat.c_str(),  tmIn.tm_yday, tmIn.tm_hour, tmIn.tm_min, tmIn.tm_sec);
	return tmp;
}

std::string TimeTable::mGetInfo()
{
	return mGetInfo(mGetCurrentTime("%a"));
}

std::string TimeTable::mGetInfo(const std::string& week)
{
	std::vector<std::string> Infos;
	Infos.clear();
	mGetTodayMoreInfo(Infos, week);
	static int count{ 0 };
	static int currentItem{ 0 };
	const int changeAfterTimes{ 10 };
	if (Infos.empty()) {
		return "";
	}

	if (count >= changeAfterTimes) {
		if (currentItem >= Infos.size()-1) {
			currentItem = 0;
		}
		else
		{
			++currentItem;
		}
		count = 0;
	}
	else
	{
		++count;
	}
	return Infos[currentItem];
}

int TimeTable::mGetLesson(std::vector<std::string>& input, const std::string& week)
{
	std::vector<Lesson> lessons;
	mGetLesson(lessons, week);
	for (const auto& i : lessons) {
		input.push_back(i.GetValue("\t"));
	}
	return 1;
}

int TimeTable::mGetLesson(std::vector<Lesson>& input, const std::string& week)
{
	Json::Value root = GetRootJsonValue(mLessonInfoPath);
	const Json::Value Lessons = root[week]["Lessons"];
	for (unsigned int i = 0; i < Lessons.size(); ++i) {
		Lesson lesson{ week, Lessons[i][0].asString(), atoi(Lessons[i][1].asString().c_str()), atoi(Lessons[i][2].asString().c_str()) };
		input.push_back(lesson);
	}
	return 1;

}
