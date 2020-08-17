#include <bb/BbServer.hpp>



class TestService : public BbService
{
public:
	TestService()
	{
		bindGet("/json", TestService::json);
		bindGet("/html", TestService::html);
		bindGet("/downloadFile", TestService::downloadFile);

		bindPost("/post", TestService::post);
	}

public:
	static void json(BbRequest& request, BbResponse& response)
	{
		response.replyJson("{\"name\":\"huangshangbin\"}");
	}

	static void html(BbRequest& request, BbResponse& response)
	{
		string htmlStr = "<html><body>\
			<a href=\"http://localhost:5000\">home</a>\
			<img src=\"test.jpg\" />\
			</body></html>";

		response.replyHtml(htmlStr);
	}

	static void downloadFile(BbRequest& request, BbResponse& response)
	{
		response.replyFile("E:\\test.lib");
	}

	static void post(BbRequest& request, BbResponse& response)
	{
		cout <<"post body:"<< endl << request.m_body << endl <<endl;
	}
};

int main()
{
	BbServer server;

	server.get("/", [](BbRequest& request, BbResponse& response) {
		response.replyText("direct bind Get");
	});

	server.post("/", [](BbRequest& request, BbResponse& response) {
		response.replyText("direct bind Post");
	});

	server.addService(new TestService());

	server.setMostThreadCount(6);
	server.listen("127.0.0.1", 5000);

	return 0;
}
