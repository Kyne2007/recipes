set(net_SRCS
  acceptor.cc
  buffer.cc
  channel.cc
  connector.cc
  event_loop.cc
  event_loop_thread.cc
  event_loop_thread_pool.cc
  inet_address.cc
  poller.cc
  socket.cc
  sockets_ops.cc
  #TcpClient.cc
  tcp_connection.cc
  tcp_server.cc
  timer.cc
  timer_queue.cc
  )

add_library(mouse_net ${net_SRCS})
target_link_libraries(mouse_net mouse_base)

