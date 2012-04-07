/*
 * Copyright (c) 2002-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "connect_timed.h"

int connect_timed(int sockfd, const struct sockaddr *saptr,
		  socklen_t salen, int nsecs){
  /*
   * This function imitates a similar function in unp1, p.411; including
   * the comment at the end of p.412 regarding the second call to conect().
   */
  int flags;
  int n;
  int error;
  fd_set rset, wset;
  struct timeval tval;

  assert(nsecs >= 0);
  if(nsecs < 0){
    errno = EINVAL;
    return(-1);
  }

  if((flags = fcntl(sockfd, F_GETFL, 0)) == -1)
    return(-1);

  if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    return(-1);

  if((n = connect(sockfd, saptr, salen)) < 0){
    if(errno != EINPROGRESS)
      return(-1);
  } else if(n == 0)
    goto end;	/* connect completed immediately */

  FD_ZERO(&rset);
  FD_SET(sockfd, &rset);
  wset = rset;
  tval.tv_sec = nsecs;
  tval.tv_usec = 0;
  
  if((n = select(sockfd + 1, &rset, &wset, NULL, nsecs ? &tval : NULL)) <= 0) {
    if(n == 0)
      errno = ETIMEDOUT;
    
    return(-1);
  }

  /*
   * If select returned no-error it could be because (i) there is some data,
   * or (ii) because the connection failed. Check the connect again.
   */
  error = errno;
  if(FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)){
    if((n = connect(sockfd, saptr, salen)) < 0){
      if(errno != EISCONN){
	errno = error;
	return(-1);
      }
    }
  }
  
 end:

  /* restore file status flags */
  if(fcntl(sockfd, F_SETFL, flags) == -1)
    return(-1);

  return(0);
}
