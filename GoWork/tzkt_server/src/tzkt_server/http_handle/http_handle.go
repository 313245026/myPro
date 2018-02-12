package http_handle

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"sync"
	"time"
	"tzkt_server/db_handle"
)

type rspStruct struct {
	Status    int64                  `json:"status"`
	Msg       string                 `json:"msg"`
	MessageID string                 `json:"messageId"`
	Data      map[string]interface{} `json:"data"`
}

//data{a:[],b:{}}

func StartServer() {
	s := &http.Server{
		Addr:           ":8080",
		Handler:        nil,
		ReadTimeout:    8 * time.Second,
		WriteTimeout:   8 * time.Second,
		MaxHeaderBytes: 1 << 20,
	}
	initHTTpHandle()
	log.Fatal(s.ListenAndServe())
}

func query(rsp http.ResponseWriter, req *http.Request) {
	io.WriteString(rsp, "Hello, world!")

}
func queryListenRecord(rsp http.ResponseWriter, req *http.Request) {

}
func addListenRecord(rsp http.ResponseWriter, req *http.Request) {

}
func getRealtime(rsp http.ResponseWriter, req *http.Request) {

}
func getRole(rsp http.ResponseWriter, req *http.Request) {

}
func checkCollectCourse(rsp http.ResponseWriter, req *http.Request) {

}
func queryQqGroups(rsp http.ResponseWriter, req *http.Request) {

}
func suggest(rsp http.ResponseWriter, req *http.Request) {

}

func queryChatperForFront(rsp http.ResponseWriter, req *http.Request) {

}
func getByStudent(rsp http.ResponseWriter, req *http.Request) {

}
func getByTeacher(rsp http.ResponseWriter, req *http.Request) {

}

func modify_chatroom_announcement(rsp http.ResponseWriter, req *http.Request) {

}
func push_stream_fail(rsp http.ResponseWriter, req *http.Request) {

}
func update_function_config(rsp http.ResponseWriter, req *http.Request) {

}
func get_others_info(rsp http.ResponseWriter, req *http.Request) {

}
func in_channel(rsp http.ResponseWriter, req *http.Request) {

}
func reject_invite(rsp http.ResponseWriter, req *http.Request) {

}
func accept_invite(rsp http.ResponseWriter, req *http.Request) {

}
func invite_student(rsp http.ResponseWriter, req *http.Request) {

}
func kick_student(rsp http.ResponseWriter, req *http.Request) {

}
func out_channel(rsp http.ResponseWriter, req *http.Request) {

}
func agree_in_channel(rsp http.ResponseWriter, req *http.Request) {

}
func raise_hand(rsp http.ResponseWriter, req *http.Request) {

}
func query_raiseHandList(rsp http.ResponseWriter, req *http.Request) {

}
func batch_query(rsp http.ResponseWriter, req *http.Request) {

}
func add_or_modify(rsp http.ResponseWriter, req *http.Request) {

}
func send_flower(rsp http.ResponseWriter, req *http.Request) {

}
func get_publish_type(rsp http.ResponseWriter, req *http.Request) {

}
func activate(rsp http.ResponseWriter, req *http.Request) {

}
func cancel(rsp http.ResponseWriter, req *http.Request) {

}
func get_login_sign(rsp http.ResponseWriter, req *http.Request) {

}

func teacherInfo(rsp http.ResponseWriter, req *http.Request) {

}
func queryByTeachQuality(rsp http.ResponseWriter, req *http.Request) {

}

func rspError(err error, rsp http.ResponseWriter) {
	var rspS rspStruct
	rspS.Status = -1
	rspS.MessageID = "123"
	rspS.Msg = err.Error()
	rspStr, _ := json.Marshal(rspS)
	io.WriteString(rsp, string(rspStr))
}

func loginVerify(rsp http.ResponseWriter, req *http.Request) {
	account := req.FormValue("account")
	password := req.FormValue("password")
	token, err := db_handle.GetToken(account, password)
	if err == nil {
		var rspS rspStruct
		rspS.Status = 0
		rspS.Data = make(map[string]interface{})
		rspS.Data["token"] = token
		rspStr, _ := json.Marshal(rspS)
		io.WriteString(rsp, string(rspStr))
	} else {
		rspError(err, rsp)
	}
}

func getPersonalInfo(rsp http.ResponseWriter, req *http.Request) {
	token := req.FormValue("token")
	account, headImg, qq, email, phone, nickname, uid, err := db_handle.GetPersonalInfo(token)
	if err == nil {
		var rspS rspStruct
		rspS.Data = make(map[string]interface{})
		personalInfo := make(map[string]interface{})
		personalInfo["account"] = account
		personalInfo["headImg"] = headImg
		personalInfo["qq"] = qq
		personalInfo["email"] = email
		personalInfo["phone"] = phone
		personalInfo["nickname"] = nickname
		personalInfo["uid"] = uid
		rspS.Data["userDetail"] = personalInfo
		rspStr, _ := json.Marshal(rspS)
		io.WriteString(rsp, string(rspStr))
	} else {
		rspError(err, rsp)
	}
}
func queryByTeacher(rsp http.ResponseWriter, req *http.Request) {
	token := req.FormValue("token")
	mapInfo, err := db_handle.QueryByTeacher(token)
	if err == nil {
		var rspS rspStruct
		rspS.Data = make(map[string]interface{})
		rspS.Data["count"] = len(mapInfo)
		rspS.Data["courses"] = mapInfo
		rspStr, _ := json.Marshal(rspS)
		io.WriteString(rsp, string(rspStr))
	} else {
		rspError(err, rsp)
	}
}

func queryByStudentr(rsp http.ResponseWriter, req *http.Request) {
	token := req.FormValue("token")
	mapInfo, err := db_handle.QueryByStudentr(token)
	if err == nil {
		var rspS rspStruct
		rspS.Data = make(map[string]interface{})
		rspS.Data["count"] = len(mapInfo)
		rspS.Data["courses"] = mapInfo
		rspStr, _ := json.Marshal(rspS)
		io.WriteString(rsp, string(rspStr))
	} else {
		rspError(err, rsp)
	}
}
func editInfoForTeacher(rsp http.ResponseWriter, req *http.Request) {
	var rspS rspStruct
	//	rspS.Data = make(map[string]interface{})
	rspStr, _ := json.Marshal(rspS)
	io.WriteString(rsp, string(rspStr))
}
func collectCourse(rsp http.ResponseWriter, req *http.Request) {
	token := req.PostFormValue("token")
	courseID := req.PostFormValue("courseId")
	status := req.PostFormValue("status")
	if err := db_handle.CollectCourse(token, courseID, status); err == nil {
		var rspS rspStruct
		rspS.Data = make(map[string]interface{})
		rspS.Data["result"] = 1
		rspStr, _ := json.Marshal(rspS)
		io.WriteString(rsp, string(rspStr))
	} else {
		rspError(err, rsp)
	}
}

func teacherStartClass(rsp http.ResponseWriter, req *http.Request, bStart bool) {
	token := req.FormValue("token")
	courseID := req.FormValue("courseId")
	classID := req.FormValue("classId")
	chapterID := req.FormValue("chapterId")
	if err := db_handle.TeaStart(token, courseID, classID, chapterID, bStart); err == nil {
		var rspS rspStruct
		rspStr, _ := json.Marshal(rspS)
		io.WriteString(rsp, string(rspStr))
	} else {
		rspError(err, rsp)
	}
}

/*老师上课*/
func start(rsp http.ResponseWriter, req *http.Request) {
	teacherStartClass(rsp, req, true)
}

/*老师下课*/
func stop(rsp http.ResponseWriter, req *http.Request) {
	teacherStartClass(rsp, req, false)
}

/*成员进入教室*/
func enter(rsp http.ResponseWriter, req *http.Request) {
	// token := req.FormValue("token")
	// courseID := req.FormValue("courseId")
	// classID := req.FormValue("classId")
	// chapterID := req.FormValue("chapterId")
}

var gCount int64 = 0
var startTime = time.Now().Format(time.StampMilli)

var gM *sync.Mutex = new(sync.Mutex)

func helloHandler(rsp http.ResponseWriter, req *http.Request) {
	go func(rsp http.ResponseWriter, req *http.Request) {
		time.Sleep(500 * time.Millisecond)
		io.WriteString(rsp, "Hello, world!321321")
		//gM.Lock()
		gCount++
		fmt.Println(gCount)
		//fmt.Println(startTime, time.Now().Format(time.StampMilli), gCount)
		//gM.Unlock()
	}(rsp, req)
}

/*
/api/update/userInfo 					UserBaseDataModify
/api/user/editInfoForTeacher 			UserOtherDataModify
/api/interaction/queryCollectCourse		GetCollectCourse
/api/interaction/querySignupCourse		GetSignupCourse
*/
func initHTTpHandle() {
	http.HandleFunc("/", helloHandler)
	http.HandleFunc("/hello", helloHandler)

	http.HandleFunc("/api/userInfo/mine", getPersonalInfo)
	http.HandleFunc("/api/login/standard", loginVerify)                        //登录验证获取token
	http.HandleFunc("/api/course/queryByTeacher", queryByTeacher)              //请求老师授课表boya
	http.HandleFunc("/api/course/queryByStudent", queryByStudentr)             //请求学生课程表boya
	http.HandleFunc("/api/course/getByTeacher", getByTeacher)                  // 获取老师详情boya
	http.HandleFunc("/api/course/getByStudent", getByStudent)                  //获取学生详情boya
	http.HandleFunc("/api/course/queryChatperForFront", queryChatperForFront)  //获取章节信息boya
	http.HandleFunc("/api/user/editInfoForTeacher", editInfoForTeacher)        //编辑其它人的个人数据lg
	http.HandleFunc("/api/interaction/collectCourse", collectCourse)           // 收藏/取消收藏课程lg
	http.HandleFunc("/api/system/suggest", suggest)                            //问题反馈lg
	http.HandleFunc("/api/course/queryQqGroups", queryQqGroups)                //查询qq号lg
	http.HandleFunc("/api/interaction/checkCollectCourse", checkCollectCourse) // 检查课程是否已收藏lg
	http.HandleFunc("/api/study/getRole", getRole)                             //获取角色fzw
	http.HandleFunc("/api/course/getRealtime", getRealtime)                    // 获取实现信息fzw
	http.HandleFunc("/api/course/addListenRecord", addListenRecord)            // 添加播放记录fzw
	http.HandleFunc("/api/course/queryListenRecord", queryListenRecord)        // 查询播放记录fzw
	http.HandleFunc("/api/cate/query", query)                                  //查询分类fzw
	http.HandleFunc("/api/course/queryByTeachQuality", queryByTeachQuality)    // 得到教质课程boya
	http.HandleFunc("/api/user/teacherInfo", teacherInfo)                      // 得到自己的老师信息fzw
	http.HandleFunc("/api/live/start", start)                                  // 上课psz
	http.HandleFunc("/api/live/stop", stop)                                    // 下课psz
	http.HandleFunc("/api/live/enter", enter)                                  //进入教室boya
	http.HandleFunc("/api/live/qcloud/get-login-sign", get_login_sign)         // 得到腾讯签名fzw
	http.HandleFunc("/api/live/mute/cancel", cancel)                           // 取消禁言psz
	http.HandleFunc("/api/live/mute/activate", activate)
	http.HandleFunc("/api/live/qcloud/get-publish-type", get_publish_type)                    //得到服务器的推流类型
	http.HandleFunc("/api/interaction/send-flower", send_flower)                              //送花psz
	http.HandleFunc("/api/user-tag/add-or-modify", add_or_modify)                             // 习惯用户标记fzw
	http.HandleFunc("/api/user-tag/batch-query", batch_query)                                 //批量获取用户标记fzw
	http.HandleFunc("/api/interaction/query-raiseHandList", query_raiseHandList)              // 查询麦序gqx
	http.HandleFunc("/api/interaction/raise-hand", raise_hand)                                // 学生申请连麦gqx
	http.HandleFunc("/api/interaction/agree-in-channel", agree_in_channel)                    //老师同意连麦gqx
	http.HandleFunc("/api/interaction/out-channel", out_channel)                              // 学生主动下麦gqx
	http.HandleFunc("/api/interaction/kick-student", kick_student)                            // 老师提学生下麦gqx
	http.HandleFunc("/api/interaction/invite-student", invite_student)                        // 老师邀请学生上麦gqx
	http.HandleFunc("/api/interaction/accept-invite", accept_invite)                          // 学生同意连麦gqx
	http.HandleFunc("/api/interaction/reject-invite", reject_invite)                          // 学生拒绝连麦gqx
	http.HandleFunc("/api/interaction/in-channel", in_channel)                                // 学生混流gqx
	http.HandleFunc("/api/interaction/get-others-info", get_others_info)                      // 得到别人信息 gqx
	http.HandleFunc("/api/course/update-function-config", update_function_config)             // 教室功能配置psz
	http.HandleFunc("/api/interaction/push-stream-fail", push_stream_fail)                    //推流失败psz
	http.HandleFunc("/api/course/modify-chatroom-announcement", modify_chatroom_announcement) // 修改公告psz
}
