//
// Created by stephane bourque on 2021-10-28.
//

#include "JobController.h"

namespace OpenWifi {

    void RegisterJobTypes();

    int JobController::Start() {

        RegisterJobTypes();

        if(!Running_)
            Thr_.start(*this);

        return 0;
    }

    void JobController::Stop() {
        if(Running_) {
            Running_ = false;
            Thr_.join();
        }
    }

    void JobController::run() {
        Running_ = true ;
        Utils::SetThreadName("job-controller");
        while(Running_) {
            Poco::Thread::trySleep(2000);

            std::lock_guard G(Mutex_);

            std::cout << jobs_.size() << " jobs in queue." << std::endl;

            for(auto &job:jobs_) {
                if(job!=nullptr) {
                    if(job->Started()==0 && Pool_.used()<Pool_.available()) {
                        std::cout << "Starting: " << job->Name() << "    ID:" << job->JobId() << std::endl;
                        job->Logger().information(fmt::format("Starting {}: {}",job->JobId(),job->Name()));
                        job->Start();
                        Pool_.start(*job);
                    }
                }
            }

            std::cout << Pool_.used() << " jobs running. Max jobs: " << Pool_.available() << std::endl;
            for(auto it = jobs_.begin(); it!=jobs_.end();) {
                if(*it!=nullptr && (*it)->Completed()!=0) {
                    auto tmp = it;
                    std::cout << "Completed: " << (*it)->Name() << "    ID:" << (*it)->JobId() << std::endl;
                    it = jobs_.erase(it);
                    delete *tmp;
                } else {
                    ++it;
                }
            }
        }

    }

}