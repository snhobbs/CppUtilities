#include <cstdint>
#include <array>
#include <iostream>
#include "CooperativeScheduler.h"
#if 0
int32_t HelloWorld1(void){
    std::cout << "Hello World1\n";
    return 0;
}
int32_t HelloWorld0(void){
    std::cout << "Hello World0\n";
    return 0;
}

int32_t HouseKeeping(void){
    std::cout << "House Keeping\n";
    return 0;
}

int main(void){
    CooperativeTask houseKeeping{static_cast<uint32_t>(-1),0,0,HouseKeeping, true};
    TaskLink* tl_house = new(TaskLink);
    tl_house->link = nullptr;
    tl_house->p_task = &houseKeeping;

    CooperativeTask hwtask0{0,10,0,HelloWorld0,false};
    TaskLink* tl_hw0 = new(TaskLink);
    tl_hw0->link = nullptr;
    tl_hw0->p_task = &hwtask0;

    CooperativeTask hwtask1{1,10,0,HelloWorld1,false};
    TaskLink* tl_hw1 = new(TaskLink);
    tl_hw1->link = nullptr;
    tl_hw1->p_task = &hwtask1;

    TaskScheduler taskManager;
    taskManager.AddTask(tl_house, 0);
    taskManager.AddTask(tl_hw0, 0);
    taskManager.AddTask(tl_hw1, 0);
    std::cout << "Num Tasks " << taskManager.GetNumTasks() << "\n";

    for(uint32_t i = 0; i < 1300; i++){
        taskManager.RunNextTask(i);
        TaskLink* ptl = taskManager.PopFromFree();
        if(ptl != nullptr){
            std::cout << "Free Link\n";
            delete ptl;
        }
    }
    return 0;
}
#endif
