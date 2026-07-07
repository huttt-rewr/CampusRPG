#include "Task.h"
#include <sstream>

Task::Task(const std::string& n, const std::string& d, const std::string& c,
           int er, int gr, int t)
    : name(n), description(d), condition(c),
      completed(false), accepted(false),
      expReward(er), goldReward(gr), progress(0), target(t) {}

void Task::accept() { accepted = true; }
bool Task::isCompleted() const { return completed; }
bool Task::isAccepted() const { return accepted; }

std::string Task::claimReward() {
    if (!completed) return "任务尚未完成!";
    std::ostringstream oss;
    oss << "任务完成: " << name << "! 获得 " << expReward << "EXP + " << goldReward << "G";
    return oss.str();
}

void Task::addProgress(int n) {
    if (!accepted) return;
    progress += n;
    if (progress >= target && !completed) {
        completed = true;
    }
}

std::string Task::getName() const { return name; }
std::string Task::getDesc() const { return description; }
std::string Task::getCondition() const { return condition; }
int Task::getExpReward() const { return expReward; }
int Task::getGoldReward() const { return goldReward; }
int Task::getProgress() const { return progress; }
int Task::getTarget() const { return target; }
