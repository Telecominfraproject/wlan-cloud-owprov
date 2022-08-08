FROM alpine:3.15 AS build-base

RUN apk add --update --no-cache \
    make cmake g++ git \
    unixodbc-dev postgresql-dev mariadb-dev \
    librdkafka-dev boost-dev openssl-dev \
    zlib-dev nlohmann-json \
    curl-dev

FROM build-base AS poco-build

ADD https://api.github.com/repos/AriliaWireless/poco/git/refs/tags/poco-tip-v1 version.json
RUN git clone https://github.com/AriliaWireless/poco --branch poco-tip-v1 /poco

WORKDIR /poco
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS cppkafka-build

ADD https://api.github.com/repos/AriliaWireless/cppkafka/git/refs/tags/tip-v1 version.json
RUN git clone https://github.com/AriliaWireless/cppkafka --branch tip-v1 /cppkafka

WORKDIR /cppkafka
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS fmtlib-build

ADD https://api.github.com/repos/fmtlib/fmt/git/refs/tags/9.0.0 version.json
RUN git clone https://github.com/fmtlib/fmt --branch 9.0.0 /fmtlib

WORKDIR /fmtlib
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN make
RUN make install

FROM build-base AS json-schema-validator-build

ADD https://api.github.com/repos/pboettch/json-schema-validator/git/refs/tags/2.1.0 version.json
RUN git clone https://github.com/pboettch/json-schema-validator --branch 2.1.0 /json-schema-validator

WORKDIR /json-schema-validator
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN make
RUN make install

FROM build-base AS owprov-build

ADD CMakeLists.txt build /owprov/
ADD cmake /owprov/cmake
ADD src /owprov/src
ADD .git /owprov/.git

COPY --from=poco-build /usr/local/include /usr/local/include
COPY --from=poco-build /usr/local/lib /usr/local/lib
COPY --from=cppkafka-build /usr/local/include /usr/local/include
COPY --from=cppkafka-build /usr/local/lib /usr/local/lib
COPY --from=json-schema-validator-build /usr/local/include /usr/local/include
COPY --from=json-schema-validator-build /usr/local/lib /usr/local/lib
COPY --from=fmtlib-build /usr/local/include /usr/local/include
COPY --from=fmtlib-build /usr/local/lib /usr/local/lib

WORKDIR /owprov
RUN mkdir cmake-build
WORKDIR /owprov/cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8

FROM alpine:3.15

ENV OWPROV_USER=owprov \
    OWPROV_ROOT=/owprov-data \
    OWPROV_CONFIG=/owprov-data

RUN addgroup -S "$OWPROV_USER" && \
    adduser -S -G "$OWPROV_USER" "$OWPROV_USER"

RUN mkdir /openwifi
RUN mkdir -p "$OWPROV_ROOT" "$OWPROV_CONFIG" && \
    chown "$OWPROV_USER": "$OWPROV_ROOT" "$OWPROV_CONFIG"

RUN apk add --update --no-cache librdkafka su-exec gettext ca-certificates bash jq curl \
    mariadb-connector-c libpq unixodbc postgresql-client

COPY readiness_check /readiness_check
COPY test_scripts/curl/cli /cli

COPY owprov.properties.tmpl /
COPY docker-entrypoint.sh /
COPY wait-for-postgres.sh /
RUN wget https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-ucentral-deploy/main/docker-compose/certs/restapi-ca.pem \
    -O /usr/local/share/ca-certificates/restapi-ca-selfsigned.pem

COPY --from=owprov-build /owprov/cmake-build/owprov /openwifi/owprov
COPY --from=cppkafka-build /cppkafka/cmake-build/src/lib/* /usr/local/lib
COPY --from=poco-build /poco/cmake-build/lib/* /usr/local/lib

EXPOSE 16005 17005 16105

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["/openwifi/owprov"]
