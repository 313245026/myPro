package db_handle

import (
	"database/sql"
	"errors"
	"log"
	"strconv"
	"strings"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

/*获得 token*/
func GetToken(account, password string) (token string, err error) {
	db, err := sql.Open("sqlite3", "./tzkt.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	stmt, err := db.Prepare("select token from user_info where account = ? and password =?")
	if err != nil {
		log.Fatal(err)
		return
	}
	defer stmt.Close()
	row := stmt.QueryRow(account, password)

	if row != nil {
		row.Scan(&token)
	}
	if token == "" {
		err = errors.New("账号或密码不存在")
	}
	return
}

/*获得个人信息*/
func GetPersonalInfo(token string) (account, headImg, qq, email, phone, nickname string, uid int, err error) {
	db, err := sql.Open("sqlite3", "./tzkt.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	stmt, err := db.Prepare("select account,uid,headImg,qq,email,phone,nickname from user_info where token = ?")
	if err != nil {
		log.Fatal(err)
		return
	}
	defer stmt.Close()
	row := stmt.QueryRow(token)
	if row != nil {
		row.Scan(&account, &uid, &headImg, &qq, &email, &phone, &nickname)
	}
	if account == "" {
		err = errors.New("token不存在")
	}
	return
}

func verfiyToken(token string) (uid int, err error) {
	db, err := sql.Open("sqlite3", "./tzkt.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	stmt, err := db.Prepare("select uid from user_info where token = ?")
	if err != nil {
		log.Fatal(err)
		return
	}
	defer stmt.Close()
	row := stmt.QueryRow(token)
	if row != nil {
		row.Scan(&uid)
	}
	if uid == 0 {
		err = errors.New("token不存在")
	}
	return
}

func getCourseIDByUID(uid int) (mapInfo []map[string]interface{}, err error) {
	db, err := sql.Open("sqlite3", "./tzkt.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()
	sqlStr := "select courseId,classId,role from user2course_info where useId = ?"
	repalse := strings.NewReplacer("?", strconv.Itoa(uid))
	sqls := repalse.Replace(sqlStr)
	rows, err := db.Query(sqls)
	if err != nil {
		log.Fatal(err)
		err = errors.New("你没有课程")
		return
	}
	for rows.Next() {
		courseInfo := make(map[string]interface{})
		id := 0
		classid := 0
		role := 0
		if err = rows.Scan(&id, &classid, &role); err != nil {
			log.Fatal(err)
			return
		}
		courseInfo["roleType"] = strconv.Itoa(role)
		courseInfo["id"] = id
		courseInfo["classId"] = classid
		courseInfo["classIdFk"] = classid
		if classid == 0 {
			courseInfo["placementStatus"] = 0
		} else {
			courseInfo["placementStatus"] = 1
		}

		mapInfo = append(mapInfo, courseInfo)
	}
	return
}

func getCourseByClassID(infoMap *[]map[string]interface{}) (err error) {
	db, err := sql.Open("sqlite3", "./tzkt.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()
	for _, v := range *infoMap {
		classid := v["classId"].(int)
		sqlStr := "select chapterTime from course_info where classId = ? order by chapterTime desc"
		rePlace := strings.NewReplacer("?", strconv.Itoa(classid))
		sqlS := rePlace.Replace(sqlStr)
		rows, err := db.Query(sqlS)
		if err != nil {
			log.Fatal(sqlS)
			log.Fatal(err)
			continue
		}
		var chapterNum int
		var endChapterNumber int
		var curTimes = time.Now().Unix()
		var startTime int64
		for rows.Next() {
			chapterNum = chapterNum + 1
			var Chaptertime int64
			rows.Scan(&Chaptertime)
			if (Chaptertime > (curTimes - 60*60*2)) && (Chaptertime < (curTimes + 60*60*2)) {
				startTime = Chaptertime
			}
			if Chaptertime < (curTimes - 60*60*2) {
				endChapterNumber = endChapterNumber + 1
			}
		}
		v["chapterNum"] = chapterNum
		v["startTime"] = startTime
		v["endChapterNumber"] = endChapterNumber

		stmt, err := db.Prepare("select mainTeacher,typeDicFk,cover from course_info where classId = ? limit 1")
		if err != nil {
			log.Fatal(err)
		}
		row := stmt.QueryRow(classid)
		var mainTeacher = ""
		var typeDicFk = 0
		var cover = ""
		if row != nil {
			row.Scan(&mainTeacher, &typeDicFk, &cover)
			v["mainTeacher"] = mainTeacher
			v["typeDicFk"] = typeDicFk
			v["cover"] = cover
		}
	}
	return
}

/*获得老师课程*/
func QueryByTeacher(token string) (courseMapinfo []map[string]interface{}, err error) {
	uid, err := verfiyToken(token)
	if err != nil {
		return
	}
	if courseMapinfo, err = getCourseIDByUID(uid); err == nil {
		err = getCourseByClassID(&courseMapinfo)
	}
	return
}

/*获得学生课程*/
func QueryByStudentr(token string) (courseMapinfo []map[string]interface{}, err error) {
	uid, err := verfiyToken(token)
	if err != nil {
		return
	}
	if courseMapinfo, err = getCourseIDByUID(uid); err == nil {
		err = getCourseByClassID(&courseMapinfo)
	}
	return
}

/*收藏操作*/
func CollectCourse(token, courseID, status string) (err error) {
	db, err := sql.Open("sqlite3", "./tzkt.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()
	if uid, err := verfiyToken(token); err == nil {
		stat := false
		if status == "1" {
			stat = true
		}
		sqlstr := "update user2course_info set collectCourse =%1 where useId=%2 and courseId = %3"
		rplace := strings.NewReplacer("%1", strconv.FormatBool(stat), "%2", strconv.Itoa(uid), "%3", courseID)
		sqls := rplace.Replace(sqlstr)
		_, err = db.Exec(sqls)
		if err != nil {
			log.Fatal(sqls)
			log.Fatal(err)
		}
	}
	return
}

/*老师上课*/
func TeaStart(token, courseID, classID, chapterID string, bStart bool) (err error) {
	db, err := sql.Open("sqlite3", "./tzkt.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()
	if uid, err := verfiyToken(token); err == nil {
		sqlstr := "update user2course_info set isLiving =%1 where useId=%2 and courseId = %3 and classId =%4 and chapterId=%5"
		rplace := strings.NewReplacer("%1", strconv.FormatBool(bStart), "%2", strconv.Itoa(uid),
			"%3", courseID, "%4", classID, "%5", chapterID)
		sqls := rplace.Replace(sqlstr)
		_, err = db.Exec(sqls)
		if err != nil {
			log.Fatal(sqls)
			log.Fatal(err)
		}
	}
	return
}

func getMemRole(uid int, courseID, classID, chapterID string) (role int, err error) {
	db, err := sql.Open("sqlite3", "./tzkt.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	stmt, err := db.Prepare("select name from foo where useId = ? and courseId = ? and classId = ?")
	if err != nil {
		log.Fatal(err)
	}
	defer stmt.Close()
	err = stmt.QueryRow(uid, courseID, classID).Scan(&role)
	if err != nil {
		log.Fatal(err)
	}
	return
}

/*成员进入教室*/
func MemberInClass(token, courseID, classID, chapterID string) (rspInfo map[string]interface{}, err error) {

	if uid, err := verfiyToken(token); err == nil {
		if role, err := getMemRole(uid, courseID, classID, chapterID); err == nil {
			if role != 0 {
				//说明是管理员-需要加入管理员列表
			}
		}
	}
	return
}

/*
	// db, err := sql.Open("sqlite3", "./foo.db")
	// if err != nil {
	// 	log.Fatal(err)
	// }
	// defer db.Close()

	// _, err = db.Exec("insert into userinfo(usename, useid,courid) values('user1', 11111,111122),('user2', 22222,2222233),('user3', 33333,3333344)")
	// if err != nil {
	// 	log.Fatal(err)
	// }
func main() {
	os.Remove("./foo.db")

	db, err := sql.Open("sqlite3", "./foo.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	sqlStmt := `
	create table foo (id integer not null primary key, name text);
	delete from foo;
	`
	_, err = db.Exec(sqlStmt)
	if err != nil {
		log.Printf("%q: %s\n", err, sqlStmt)
		return
	}

	tx, err := db.Begin()
	if err != nil {
		log.Fatal(err)
	}
	stmt, err := tx.Prepare("insert into foo(id, name) values(?, ?)")
	if err != nil {
		log.Fatal(err)
	}
	defer stmt.Close()
	for i := 0; i < 100; i++ {
		_, err = stmt.Exec(i, fmt.Sprintf("こんにちわ世界%03d", i))
		if err != nil {
			log.Fatal(err)
		}
	}
	tx.Commit()

	rows, err := db.Query("select id, name from foo")
	if err != nil {
		log.Fatal(err)
	}
	defer rows.Close()
	for rows.Next() {
		var id int
		var name string
		err = rows.Scan(&id, &name)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Println(id, name)
	}
	err = rows.Err()
	if err != nil {
		log.Fatal(err)
	}

	stmt, err = db.Prepare("select name from foo where id = ?")
	if err != nil {
		log.Fatal(err)
	}
	defer stmt.Close()
	var name string
	err = stmt.QueryRow("3").Scan(&name)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println(name)

	_, err = db.Exec("delete from foo")
	if err != nil {
		log.Fatal(err)
	}

	_, err = db.Exec("insert into foo(id, name) values(1, 'foo'), (2, 'bar'), (3, 'baz')")
	if err != nil {
		log.Fatal(err)
	}

	rows, err = db.Query("select id, name from foo")
	if err != nil {
		log.Fatal(err)
	}
	defer rows.Close()
	for rows.Next() {
		var id int
		var name string
		err = rows.Scan(&id, &name)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Println(id, name)
	}
	err = rows.Err()
	if err != nil {
		log.Fatal(err)
	}
}

*/
