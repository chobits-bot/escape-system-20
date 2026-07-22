# ---- Run Stage ----
FROM alpine:3.19

RUN apk add --no-cache ca-certificates tzdata
ENV TZ=Asia/Shanghai

WORKDIR /app

COPY aiot-server .
COPY manifest/config/ manifest/config/

EXPOSE 8080

CMD ["./aiot-server"]
