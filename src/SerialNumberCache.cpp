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
		std::lock_guard		G(M_);

		auto Hint = DeviceTypeDictionary_.find(DeviceType);
		int     Index;
		if(Hint == end(DeviceTypeDictionary_)) {
		    Index = DeviceTypeIndex_;
		    DeviceTypeDictionary_[DeviceType] = DeviceTypeIndex_++;
		} else {
		    Index = Hint->second;
		}

		uint64_t SN = std::stoull(S,0,16);
		if(std::find_if(SNs_.begin(),SNs_.end(),[SN](DeviceTypeCacheEntry &E) { return E.SerialNumber == SN; }) == SNs_.end()) {
		    auto NE = DeviceTypeCacheEntry{ .SerialNumber = SN, .DeviceType = Index };
			if(SNs_.size()+1 == SNs_.capacity())
				SNs_.resize(SNs_.capacity()+2000);
			SNs_.push_back(NE);
			std::sort(SNs_.begin(),SNs_.end(), [](const DeviceTypeCacheEntry &E1, const DeviceTypeCacheEntry &E2) { return E1.SerialNumber < E2.SerialNumber ; });
		}
	}

	void SerialNumberCache::DeleteSerialNumber(const std::string &S) {
		std::lock_guard		G(M_);

		uint64_t SN = std::stoull(S,0,16);
		auto It = std::find_if(SNs_.begin(),SNs_.end(),[SN](const DeviceTypeCacheEntry &E) { return E.SerialNumber == SN; });
		if(It != SNs_.end()) {
			SNs_.erase(It);
		}
	}

	void SerialNumberCache::FindNumbers(const std::string &S, uint HowMany, std::vector<uint64_t> &A) {
		std::lock_guard		G(M_);

		if(S.length()==12) {
			uint64_t SN = std::stoull(S,0,16);
			auto It = std::find_if(SNs_.begin(),SNs_.end(), [SN](const DeviceTypeCacheEntry &E) { return E.SerialNumber == SN; } );
			if(It != SNs_.end()) {
				A.push_back(It->SerialNumber);
			}
		} else if (S.length()<12){
			std::string SS{S};
			SS.insert(SS.end(), 12 - SS.size(), '0');
			uint64_t SN = std::stoull(SS,0,16);

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
	    uint64_t SN = std::stoull(SerialNumber,0,16);
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