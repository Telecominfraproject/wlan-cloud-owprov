//
// Created by stephane bourque on 2021-08-11.
//

#include "SerialNumberCache.h"
#include <mutex>

#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {

	int SerialNumberCache::Start() {
		return 0;
	}

	void SerialNumberCache::Stop() {
	}

	void SerialNumberCache::AddSerialNumber(const std::string &S, const std::string &DeviceType) {
		std::lock_guard		G(Mutex_);

		auto Hint = DeviceTypeDictionary_.find(DeviceType);
		int     Index;
		if(Hint == end(DeviceTypeDictionary_)) {
		    Index = DeviceTypeIndex_++;
		    DeviceTypeDictionary_[DeviceType] = Index;
		} else {
		    Index = Hint->second;
		}

        uint64_t SN = std::stoull(S, nullptr, 16);
        auto match_fun = [=](const DeviceTypeCacheEntry &E) { return E.SerialNumber == SN; };
        auto lower_fun = [=](const DeviceTypeCacheEntry &E1, const DeviceTypeCacheEntry &E2) { return E1.SerialNumber < E2.SerialNumber; };

		if(std::find_if(SNs_.begin(),SNs_.end(),match_fun ) == SNs_.end()) {
            auto NewEntry = DeviceTypeCacheEntry{ .SerialNumber = SN, .DeviceType = Index };
            auto insertion_point = std::lower_bound(begin(SNs_), end(SNs_), NewEntry, lower_fun);
            SNs_.insert( insertion_point, NewEntry );
		}
	}

	void SerialNumberCache::DeleteSerialNumber(const std::string &S) {
		std::lock_guard		G(Mutex_);

		uint64_t SN = std::stoull(S, nullptr,16);
		auto It = std::find_if(SNs_.begin(),SNs_.end(),[SN](const DeviceTypeCacheEntry &E) { return E.SerialNumber == SN; });
		if(It != SNs_.end()) {
			SNs_.erase(It);
		}
	}

	void SerialNumberCache::FindNumbers(const std::string &S, uint HowMany, std::vector<uint64_t> &A) {
		std::lock_guard		G(Mutex_);

		if(S.length()==12) {
			uint64_t SN = std::stoull(S, nullptr, 16);
			auto It = std::find_if(SNs_.begin(),SNs_.end(), [SN](const DeviceTypeCacheEntry &E) { return E.SerialNumber == SN; } );
			if(It != SNs_.end()) {
				A.push_back(It->SerialNumber);
			}
		} else if (S.length()<12){
			std::string SS{S};
			SS.insert(SS.end(), 12 - SS.size(), '0');
			uint64_t SN = std::stoull(SS, nullptr, 16);

			auto LB = std::lower_bound(SNs_.begin(),SNs_.end(),SN, [](const DeviceTypeCacheEntry &E1,uint64_t V) { return E1.SerialNumber < V ; });
			if(LB!=SNs_.end()) {
				for(;LB!=SNs_.end() && HowMany;++LB,--HowMany) {
					std::string TSN = Utils::int_to_hex(LB->SerialNumber);
					if(S == TSN.substr(0,S.size())) {
						A.emplace_back(LB->SerialNumber);
					} else {
						break;
					}
				}
			}
		}
	}

	bool SerialNumberCache::FindDevice(const std::string &SerialNumber, std::string & DeviceType) {
        std::lock_guard		G(Mutex_);
	    uint64_t SN = std::stoull(SerialNumber, nullptr, 16);
	    auto It = std::find_if(SNs_.begin(),SNs_.end(),[SN](const DeviceTypeCacheEntry &E) { return E.SerialNumber == SN; });
	    if(It != SNs_.end()) {
	        for(const auto &i:DeviceTypeDictionary_)
	            if(i.second==It->DeviceType) {
	                DeviceType = i.first;
	                return true;
	            }
	    }
        return false;
	}

}