#ifndef TASK_H
#define TASK_H

#include <string>

class Task {
private:
    std::string name;
    std::string description;
    std::string condition;
    bool completed;
    bool accepted;
    int expReward;
    int goldReward;
    int progress;
    int target;

public:
    Task(const std::string& name, const std::string& desc,
         const std::string& condition, int expR, int goldR, int target = 3);

    void accept();
    bool isCompleted() const;
    bool isAccepted() const;
    std::string claimReward();  // returns reward text
    void addProgress(int n = 1);

    std::string getName() const;
    std::string getDesc() const;
    std::string getCondition() const;
    int getExpReward() const;
    int getGoldReward() const;
    int getProgress() const;
    int getTarget() const;
};

#endif
