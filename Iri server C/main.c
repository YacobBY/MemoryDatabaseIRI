#include "mongoose.h"
#include "sensors.h"
/*Classes Explanation:

sensors.h contains the structs which each sensor consists of. These structs are made filled in sensors.c

sensors.c contains the code to generate the database values, and convert them to Json using jWrite functions. The main function used for this is "generateSensors".

jWrite.c and jWrite.h contain the functions to convert the values from the sensor structs to valid Json in an object oriented way.

mongoose.c and mongoose.h contain the functions for the webserver
*/

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;


//char jsonPostedString[300] = "1,23,90,2,20,85"; is de huidige parameter string die in de functie meegaat, moet vervangen worden met de input van de Java JSON

char sensorJson[500000];

static void handle_sensors_call(struct mg_connection *nc, struct http_message *hm) {

	//Declares the final string that's going to be send to the server (sensorJson)
	char jsonPostedString[100];
	mg_get_http_var(&hm->query_string, "sensors", jsonPostedString, sizeof(jsonPostedString));
	printf("json posted string: %s\n", jsonPostedString);

	//Main function of sensors.c that generates all the values, changed boundaries based on the received string and then returns a Json string with the database values.
	char defaultPostedString[100] = "a";
	if (jsonPostedString[0] == '\0') {
		generateSensors(&sensorJson, &defaultPostedString);
	} else {
		generateSensors(&sensorJson, &jsonPostedString);
	}

	//Pefines the type of message being sent as Json
	mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Type: application/json\r\n\r\n");

	//Puts the the (now) Json string "sensorJson" in the message buffer
	mg_printf_http_chunk(nc, "{\"sensors\":[\n %s]}", sensorJson);

	//Sends the message buffer
	mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
	struct http_message *hm = (struct http_message *) ev_data;

	switch (ev) {
	case MG_EV_HTTP_REQUEST:
		if (mg_vcmp(&hm->uri, "/api/v1/sensors") == 0) {
			handle_sensors_call(nc, hm); /* Handle RESTful call */
		}
		else if (mg_vcmp(&hm->uri, "/printcontent") == 0) {
			char buf[100] = { 0 };
			memcpy(buf, hm->body.p,
				sizeof(buf) - 1 < hm->body.len ? sizeof(buf) - 1 : hm->body.len);
		}
		else {
			mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
		}
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[]) {

	
	struct mg_mgr mgr;
	struct mg_connection *nc;
	struct mg_bind_opts bind_opts;
	int i;
	char *cp;
	const char *err_str;

	mg_mgr_init(&mgr, NULL);

	/* Use current binary directory as document root */
	if (argc > 0 && ((cp = strrchr(argv[0], DIRSEP)) != NULL)) {
		*cp = '\0';
		s_http_server_opts.document_root = argv[0];
	}

	/* Process command line options to customize HTTP server */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-D") == 0 && i + 1 < argc) {
			mgr.hexdump_file = argv[++i];
		}
		else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
			s_http_server_opts.document_root = argv[++i];
		}
		else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
			s_http_port = argv[++i];
		}
		else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
			s_http_server_opts.auth_domain = argv[++i];
		}
		else if (strcmp(argv[i], "-P") == 0 && i + 1 < argc) {
			s_http_server_opts.global_auth_file = argv[++i];
		}
		else if (strcmp(argv[i], "-A") == 0 && i + 1 < argc) {
			s_http_server_opts.per_directory_auth_file = argv[++i];
		}
		else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
			s_http_server_opts.url_rewrites = argv[++i];
#if MG_ENABLE_HTTP_CGI
		}	
		else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
			s_http_server_opts.cgi_interpreter = argv[++i];
#endif
#if MG_ENABLE_SSL
		}
		else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
			ssl_cert = argv[++i];
#endif
		}
		else {
			fprintf(stderr, "Unknown option: [%s]\n", argv[i]);
			exit(1);
		}
	}

	/* Set HTTP server options */
	memset(&bind_opts, 0, sizeof(bind_opts));
	bind_opts.error_string = &err_str;
#if MG_ENABLE_SSL
	if (ssl_cert != NULL) {
		bind_opts.ssl_cert = ssl_cert;
	}
#endif
	nc = mg_bind_opt(&mgr, s_http_port, ev_handler, bind_opts);
	if (nc == NULL) {
		fprintf(stderr, "Error starting server on port %s: %s\n", s_http_port,
			*bind_opts.error_string);
		exit(1);
	}

	mg_set_protocol_http_websocket(nc);
	s_http_server_opts.enable_directory_listing = "yes";

	printf("Starting RESTful server on port %s, serving %s\n", s_http_port,
		s_http_server_opts.document_root);
	for (;;) {
		mg_mgr_poll(&mgr, 1000);
	}
	mg_mgr_free(&mgr);

	return 0;
}
