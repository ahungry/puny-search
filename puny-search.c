// Copyright (C) 2015,2016  Matthew Carter <m@ahungry.com>

// Author: Matthew Carter <m@ahungry.com>
// Maintainer: Matthew Carter <m@ahungry.com>
// URL: https://github.com/ahungry/puny-search
// Version: 0.0.0

// License:

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Commentary:

// Basic socket implementation taken from (GNU Documentation License v2)
// http://rosettacode.org/wiki/Echo_server#C

// A guide with more info on network/socket programming in C
// http://beej.us/guide/bgnet/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "config.h"

#define MAX_ENQUEUED 20
#define BUF_LEN 512
#define MAX_HTML_FILE_SIZE 8192
#define PORT_STR "12321"

/**
 * Send out the successful HTTP header
 *
 * @param int csock The client socket to write to
 * @return void
 */
void http_send_header_success (int csock)
{
  (void) write (csock, "HTTP/1.1 200 OK\n", 16);
  (void) write (csock, "Content-Type: text/html\n", 24);
  (void) write (csock, "Connection: close\n\n", 19);
}

/**
 * Read in and write out the contents of FILE_NAME to the client socket CSOCK.
 *
 * @param char file_name[10] The input file_name to parse out
 * @param int csock The client socket to write to
 * @return void
 */
void http_send_file_contents (char file_name[10], int csock)
{
  int i = 0;
  char ch, content[MAX_HTML_FILE_SIZE];

  FILE *fp;

  fp = fopen (file_name, "r");

  if (fp == NULL)
    {
      perror ("Error reading HTML template file\n");
      exit (EXIT_FAILURE);
    }

  while ( (ch = fgetc (fp) ) != EOF)
    {
      content[i++] = ch;
    }

  content[i] = '\0';
  fclose (fp);

  printf ("About to send http file content...\n");
  (void) write (csock, content, strlen (content));
}

/**
 * Parse out the HTTP method
 *
 * This typically always comes first, such as:
 *
 *   GET /something HTTP/1.1
 *
 * @param char *buf The buffer containing the HTTP client headers
 * @param char *method The parsed out http method value
 * @return int i The ending character from the parsing
 */
int http_parse_method (char *buf, char *method)
{
  int i, mi = 0;

  for (i = 0; i < (int) strlen (buf); i++)
    {
      if (' ' == buf[i])
        {
          method[mi] = '\0';
          break;
        }

      method[mi++] = buf[i];
    }

  printf ("Method: %s\n", method);

  return i;
}

/**
 * Break the query string apart based on slashes (into a multi-dimensional array)
 *
 * Currently does not support urlencoded values, only ASCII characters
 *
 * @param int i The initial buffer position to read from given a previous read to headers call
 * @param char *buf The user input buffer
 * @param char uri[][] The multi-dimensional array of parsed out uri components
 * @return int Total count of URI components/segments
 */
int http_parse_query_string (int i, char *buf, char uri[10][256])
{
  int slot = 0, ui = 0;

  // Read the query string after the method
  for (i = i + 2; i < (int) strlen (buf); i++)
    {
      // Done reading it, so abort
      if (' ' == buf[i])
        {
          uri[slot][ui] = '\0';
          break;
        }
      else if ('/' == buf[i])
        {
          // Increment to next slot and terminate previous slot
          uri[slot][ui] = '\0';
          ui = 0;
          i++;
          slot++;
        }

      // Otherwise, just keep on filling it out
      uri[slot][ui++] = buf[i];
    }

  printf ("Slots: %d\n", slot);

  for (i = 0; i <= slot; i++)
    {
      printf ("Uri: %s\n", uri[i]);
    }

  return slot;
}

/* ------------------------------------------------------------ */
/* How to clean up after dead child processes                   */
/* ------------------------------------------------------------ */
void wait_for_zombie(int s)
{
  while(waitpid(-1, NULL, WNOHANG) > 0) ;
}

/* ------------------------------------------------------------ */
/* Core of implementation of a child process                    */
/* ------------------------------------------------------------ */
void http_send_client_response(int csock)
{
  char buf[BUF_LEN];
  char uri[10][256];
  char method[20];
  int i = 0, slot = 0;

  read (csock, buf, BUF_LEN);

  i = http_parse_method (buf, method);
  slot = http_parse_query_string (i, buf, uri);

  FILE *fp;
  char path[1035];
  char cmd[256];

  http_send_header_success (csock);

  if (strcmp (method, "POST") == 0)
    {
      printf ("Received a post\n");
      strcpy (cmd, OPEN_FILE_CMD);

      for (i = 0; i < slot; i++)
        {
          strcat (cmd, "/");
          strcat (cmd, uri[i]);
        }

      strcat (cmd, "/");
      system (cmd);
    }
  else if (strcmp (method, "PUT") == 0)
    {
      printf ("Received a put\n");
      strcpy (cmd, STAT_FILE_CMD);

      for (i = 0; i <= slot; i++)
        {
          strcat (cmd, "/");
          strcat (cmd, uri[i]);
        }

      fp = popen (cmd, "r");

      if (fp == NULL)
        {
          fprintf (stderr, "Failed to popen\n");
          exit (1);
        }

      while (fgets (path, sizeof (path) - 1, fp) != NULL)
        {
          (void) write (csock, path, strlen (path));
        }

      pclose (fp);
    }
  else if (strcmp (method, "PATCH") == 0)
    {
      printf ("Received a patch\n");
      strcpy (cmd, FILE_FILE_CMD);

      for (i = 0; i <= slot; i++)
        {
          strcat (cmd, "/");
          strcat (cmd, uri[i]);
        }

      fp = popen (cmd, "r");

      if (fp == NULL)
        {
          fprintf (stderr, "Failed to popen\n");
          exit (1);
        }

      while (fgets (path, sizeof (path) - 1, fp) != NULL)
        {
          (void) write (csock, path, strlen (path));
        }

      pclose (fp);
    }
  else
    {
      printf ("Received a get\n");

      if (strlen (uri[0]) == 0)
        {
          // Here we are just sending out the basic page structure
          http_send_file_contents ("layout.html", csock);
        }
      else
        {
          // Here we are looking up a file with locate
          strcpy(cmd, LOCATE_CMD);

          for (i = 0; i <= slot; i++)
            {
              strcat (cmd, " ");
              strcat (cmd, uri[i]);
            }

          printf ("Run command: %s\n", cmd);
          fp = popen (cmd, "r");

          if (fp == NULL)
            {
              fprintf (stderr, "Failed to popen\n");
              exit (1);
            }

          while (fgets (path, sizeof (path) - 1, fp) != NULL)
            {
              (void) write (csock, "\n<div class='locate-match'>", 28);
              (void) write (csock, path, strlen (path));
              (void) write (csock, "</div>\n", 7);
            }

          pclose (fp);
        }

    }


  exit (EXIT_SUCCESS);
}

/* ------------------------------------------------------------ */
/* Core of implementation of the parent process                 */
/* ------------------------------------------------------------ */

void take_connections_forever(int ssock)
{
  for(;;) {
    struct sockaddr addr;
    socklen_t addr_size = sizeof(addr);
    int csock;

    /* Block until we take one connection to the server socket */
    csock = accept(ssock, &addr, &addr_size);

    /* If it was a successful connection, spawn a worker process to service it */
    if ( csock == -1 ) {
      perror("accept");
    } else if ( fork() == 0 ) {
      close(ssock);
      http_send_client_response(csock);
    } else {
      close(csock);
    }
  }
}

/* ------------------------------------------------------------ */
/* The server process's one-off setup code                      */
/* ------------------------------------------------------------ */

int main(int argc, char *argv[])
{
  struct addrinfo hints, *res;
  struct sigaction sa;
  int sock;
  char portno[10];

  strcpy (portno, argc > 1 ? argv[1] : PORT_STR);

  /* Look up the address to bind to */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if ( getaddrinfo(NULL, portno, &hints, &res) != 0 ) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  /* Make a socket */
  if ( (sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1 ) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  /* Arrange to clean up child processes (the workers) */
  sa.sa_handler = wait_for_zombie;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if ( sigaction(SIGCHLD, &sa, NULL) == -1 ) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  /* Associate the socket with its address */
  if ( bind(sock, res->ai_addr, res->ai_addrlen) != 0 ) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(res);

  /* State that we've opened a server socket and are listening for connections */
  if ( listen(sock, MAX_ENQUEUED) != 0 ) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  /* Serve the listening socket until killed */
  take_connections_forever(sock);
  return EXIT_SUCCESS;
}
