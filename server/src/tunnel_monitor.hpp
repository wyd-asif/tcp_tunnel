//
// Created by mabaiming on 16-12-20.
//

#ifndef TCP_TUNNEL_MONITOR_H
#define TCP_TUNNEL_MONITOR_H

#include "buffer.hpp"
#include "buffer_traffic.hpp"
#include "common.h"
#include "event_manager.h"
#include "frame.h"
#include "logger.h"

class TunnelMonitor : public EventManager {
private:
  shared_ptr<Buffer> monitorBuffer;
  string cmd;
  string readBuffer;

public:
  void init(const Addr& monitor, const string& cmd_) {
    monitorBuffer = connect(monitor.ip, monitor.port);
    if (monitorBuffer->isClosed()) {
      log_error << "failed to connect " << monitor.ip << ":" << monitor.port;
      exit(EXIT_FAILURE);
    }
    cmd = cmd_;
  }

  void onBufferCreated(shared_ptr<Buffer> buffer, const ListenInfo&) {
  }

  bool handleMonitorData() {
    if (monitorBuffer->isClosed()){
      cout << readBuffer;
      readBuffer.clear();
      exit(EXIT_SUCCESS);
      return true;
    }
    bool success = false;
    if (!cmd.empty()) {
      Frame frame;
      frame.cid = monitorBuffer->getId();
      frame.state = Frame::STATE_MONITOR_REQUEST;
      frame.message = cmd;
      if (monitorBuffer->writableSize() >= frame.getPackageSize()) {
        monitorBuffer->writeFrame(frame);
        cmd.clear();
        success = true;
      }
    }
    Frame frame;
    int n = 0;
    while((n = monitorBuffer->readFrame(frame)) > 0) {
      if (frame.state == Frame::STATE_MONITOR_RESPONSE) {
        readBuffer.append(frame.message);
      } else if (frame.state == Frame::STATE_CLOSE) {
        cout << readBuffer;
        readBuffer.clear();
        exit(EXIT_SUCCESS);
      }
      monitorBuffer->popRead(n);
      success = true;
    }
    return success;
  }

  bool exchangeData() {
    return handleMonitorData();
  }

  int idle() {
    return 0;
  }
};

#endif //TCP_TUNNEL_MONITOR_H