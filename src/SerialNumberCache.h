//
// Created by stephane bourque on 2021-08-11.
//

#ifndef UCENTRALGW_SERIALNUMBERCACHE_H
#define UCENTRALGW_SERIALNUMBERCACHE_H

#include "framework/MicroService.h"

namespace OpenWifi {
	class SerialNumberCache : public SubSystemServer {
		public:

	    struct DeviceTypeCacheEntry {
	        uint64_t        SerialNumber;
	        int             DeviceType;
	    };

        typedef std::vector<DeviceTypeCacheEntry>   SerialCacheContent;

        static auto instance() {
		    static auto instance_ = new SerialNumberCache;
		    return instance_;
		}

		int Start() override;
		void Stop() override;
		void AddSerialNumber(const std::string &SerialNumber, const std::string &DeviceType);
		void DeleteSerialNumber(const std::string &S);
		void FindNumbers(const std::string &S, uint HowMany, std::vector<uint64_t> &A);
		bool FindDevice(const std::string &SerialNumber, std::string & DeviceType);

		inline SerialCacheContent GetCacheCopy() {
		    std::lock_guard     G(Mutex_);
		    return SNs_;
		}

	  private:
		int                         DeviceTypeIndex_=0;
		uint64_t 					LastUpdate_ = 0 ;
		SerialCacheContent	        SNs_;
		std::map<std::string,int>   DeviceTypeDictionary_;

		SerialNumberCache() noexcept:
			SubSystemServer("SerialNumberCache", "SNCACHE-SVR", "serialcache")
			{
				SNs_.reserve(2000);
			}
	};

	inline auto SerialNumberCache() { return SerialNumberCache::instance(); }

} // namespace OpenWiFi

#endif // UCENTRALGW_SERIALNUMBERCACHE_H
