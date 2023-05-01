FROM --platform=linux/x86_64 gcc:4.8.5 AS build
ADD . /src
WORKDIR /src
RUN make

FROM --platform=linux/x86_64 busybox:glibc
WORKDIR /src
COPY --from=build /src/server2 .
COPY --from=build /src/client2 .
