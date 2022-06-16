#!/bin/bash

dst=~/Desktop/Dropbox/clion/last_obj_update
src=~/Desktop/Dropbox/clion
rm -rf ${dst}
mkdir ${dst}

cp ${src}/wlan-cloud-ucentralgw/src/RESTObjects/RESTAPI_GWobjects.* ${dst}/.
cp ${src}/wlan-cloud-ucentralsec/src/RESTObjects/RESTAPI_SecurityObjects.* ${dst}/.
cp ${src}/wlan-cloud-ucentralfms/src/RESTObjects/RESTAPI_FMSObjects.* ${dst}/.
cp ${src}/wlan-cloud-prov/src/RESTObjects/RESTAPI_ProvObjects.* ${dst}/.
cp ${src}/wlan-cloud-analytics/src/RESTObjects/RESTAPI_AnalyticsObjects.* ${dst}/.
cp ${src}/wlan-cloud-certportal/src/RESTObjects/RESTAPI_CertObjects.* ${dst}/.
cp ${src}/wlan-cloud-userportal/src/RESTObjects/RESTAPI_SubObjects.* ${dst}/.
cp ${src}/ucentralsim/src/RESTObjects/RESTAPI_OWLSobjects.* ${dst}/.

cp ${dst}/*.* ${src}/wlan-cloud-ucentralgw/src/RESTObjects/.
cp ${dst}/*.* ${src}/wlan-cloud-ucentralsec/src/RESTObjects/.
cp ${dst}/*.* ${src}/wlan-cloud-ucentralfms/src/RESTObjects/.
cp ${dst}/*.* ${src}/wlan-cloud-prov/src/RESTObjects/.
cp ${dst}/*.* ${src}/wlan-cloud-analytics/src/RESTObjects/.
cp ${dst}/*.* ${src}/wlan-cloud-certportal/src/RESTObjects/.
cp ${dst}/*.* ${src}/wlan-cloud-userportal/src/RESTObjects/.
cp ${dst}/*.* ${src}/ucentralsim/src/RESTObjects/.
