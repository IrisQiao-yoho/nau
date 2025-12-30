//////////////////////////////////////////////////////////////////////////
#include "httplib.h"
#include "nau_main.h"


//////////////////////////////////////////////////////////////////////////
namespace fs = std::filesystem;


//////////////////////////////////////////////////////////////////////////
int
nau_http_post(const char *addr, int port, const char *url, const char *fname)
{
	fs::path	file_to_upload = fname;

	// Step1: 检查文件是否存在
	if( !fs::exists(file_to_upload) || !fs::is_regular_file(file_to_upload) ) {

		std::cerr << "Error: File not found or is not a regular file: " << fname << std::endl;
		return -1;
	}
	// Step2: 读取文件内容
	std::ifstream ifs(fname, std::ios::binary);
	if(!ifs) {

		std::cerr << "Error: Failed to open file for reading: " << fname << std::endl;
		return -2;
	}
    std::string file_content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

	// Step3: 创建HTTP客户端
	httplib::Client cli(addr, port);
	httplib::UploadFormDataItems items = {
		{"file", file_content, file_to_upload.filename().string(), "application/octet-stream"}
	};

	// Step4: 上传文件内容
	auto res = cli.Post(url, items);
	if( res ) {

		std::cout << "Status: " << res->status << std::endl;
		std::cout << "Body:\n" << res->body << std::endl;
		if(res->status == 200) {

			std::cout << "[HTTP] : Upload File : " << fname << std::endl;
			return 0;
		} else {

			std::cout << "[HTTP] : Upload File : " << fname << ", RspCode:" << res->status << std::endl;
			return res->status;
		}
	}else {

		std::cerr << "HTTP request failed. Error: " << httplib::to_string(res.error()) << std::endl;
		return -3;
	}
	return 0;
}

void
nau_http_svr(const char *addr, int port, const char *url, const char *fpath, int	type)
{
	httplib::Server svr;

	// Step1: 设置路由
	svr.Post(url, [&](const httplib::Request& req, httplib::Response& res) {

		if( !req.form.has_file("file") ) {

			res.status = 400;
			res.set_content("Bad Request: Missing 'file' field in multipart data.", "text/plain");
			return;
		}
		const auto& file = req.form.get_file("file");
		std::string filename = file.filename;
		fs::path save_path = fs::path(fpath) / filename;

		// 保存上传文件
		std::ofstream ofs(save_path, std::ios::binary);
		if( !ofs ) {

			std::cerr << "Error: Failed to open file for writing." << std::endl;
			res.status = 500;
			res.set_content("Internal Server Error: Could not save file.", "text/plain");
			return;
		}
		ofs.write(file.content.data(), file.content.size());
		ofs.close();
		if( ofs.good() ) {

			std::cout << "[HTTP] : Recieve File : " << save_path.string() << " (" << file.content.size() << " bytes)" << std::endl;
			res.set_content("File uploaded successfully to: " + save_path.string(), "text/plain");

			// 处理上传/下发数据
			std::string fname = save_path.string();
			if(NAU_TYPE_AP == type) {
				nau_ap_upload(fname.c_str());
			}else {
				nau_gw_upload(fname.c_str());
			}
		} else {

			std::cerr << "Error: File writing failed." << std::endl;
			res.status = 500;
			res.set_content("Internal Server Error: File writing failed.", "text/plain");
		}
	});

	// 启动服务端
	std::cout << "Server listening on http://" << addr << ":" << port << url << std::endl;
	svr.listen(addr, port);
}

