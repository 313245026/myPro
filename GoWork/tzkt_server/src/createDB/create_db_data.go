package main

import (
	"crypto/md5"
	"crypto/rand"
	"database/sql"
	"encoding/base64"
	"encoding/hex"
	"fmt"
	"io"
	"log"
	mathRand "math/rand"
	"os"
	"strconv"
	"strings"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
	os.Remove("./tzkt.db")
	db, err := sql.Open("sqlite3", "./tzkt.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	if err = createTable(db); err != nil {
		fmt.Println(err.Error())
		fmt.Println("create table faile")
		return
	}
	if err = insertData(db); err != nil {
		fmt.Println(err.Error())
		fmt.Println("insert table faile")
		return
	}
}

func insertData(database *sql.DB) (err error) {
	if err = inser2Course(database); err != nil {
		return
	}
	if err = insert2UserInfo(database); err != nil {
		return
	}
	if err = insert2course2User(database); err != nil {
		return
	}
	return
}

func getRandMainTeacherName() string {
	nameRune := []rune("放大不过的切尔奇热放图不打算发呢比较库破和爸爸每次看了放大浪费大门无群若婆罗门女就办不到破IE抢不到刚过期的v女眷打包迫切而你撒地方的萨芬")

	r := mathRand.New(mathRand.NewSource(time.Now().UnixNano()))
	count := r.Intn(10)
	if count == 0 {
		count = 3
	}
	var strName string
	for index := 0; index < count; index++ {
		index1 := r.Intn(len(nameRune))
		strName = strName + string(nameRune[index1])
	}
	return strName
}

func inser2Course(database *sql.DB) (err error) {
	//create courseinfo
	var courseID int64 = 1000
	var classID int64 = 20000
	var chapterID int64 = 100000

	tx, err := database.Begin()
	if err != nil {
		log.Fatal(err)
	}
	stmt, err := tx.Prepare("INSERT INTO course_info VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);")
	if err != nil {
		log.Fatal(err)
	}

	defer stmt.Close()
	for course := 1; course <= 50; course++ {
		courseID++
		title := getRandMainTeacherName()
		for class := 1; class <= 5; class++ {
			classID++
			mainteachname := getRandMainTeacherName()
			isLiving := true //指定第一节正在直播
			for chapter := 1; chapter <= 7; chapter++ {
				chapterID++
				//插入课程列表
				chaptertile := getRandMainTeacherName()
				cover := ""
				chapterTime := time.Now().Unix()
				qqlist := ""
				manageMember := ""
				announcement := getRandMainTeacherName()
				var typeDicFk int64 = 5310
				if courseID%2 == 0 {
					typeDicFk = 5310
				} else {
					typeDicFk = 5311
				}
				_, err = stmt.Exec(courseID, classID, chapterID, manageMember, isLiving, mainteachname, title, chaptertile, cover,
					chapterTime, announcement, qqlist, typeDicFk)
				if err != nil {
					log.Fatal(err)
				}
				isLiving = false
			}
		}
	}
	tx.Commit()
	return
}

func insert2UserInfo(database *sql.DB) (err error) {
	//new account
	tx, err := database.Begin()
	if err != nil {
		log.Fatal(err)
	}
	stmt, err := tx.Prepare("INSERT INTO user_info VALUES (?,?,?,?,?,?,?,?,?,?);")
	if err != nil {
		log.Fatal(err)
	}
	defer stmt.Close()
	tmpUid := 10000
	for index := 0; index <= 20; index++ {
		var account = ""
		if index > 6 {
			account = "tea" //5701 5702 5703 3000
			if index < 10 {
				account = "tea" + strconv.Itoa(5701) + strconv.Itoa(index)
			} else if index < 13 {
				account = "tea" + strconv.Itoa(5702) + strconv.Itoa(index)
			} else if index < 17 {
				account = "tea" + strconv.Itoa(5703) + strconv.Itoa(index)
			} else if index <= 20 {
				account = "tea" + strconv.Itoa(3000) + strconv.Itoa(index)
			}
		} else {
			account = "student"
			account += strconv.Itoa(index)
		}

		password := "123456"
		qq := ""
		nickname := ""
		headImg := ""
		email := ""
		phone := ""
		token := GetGUID()
		tmpUid++
		uid := tmpUid
		role := 0
		_, err = stmt.Exec(account, password, uid, qq, nickname, headImg, email, phone, token, role)
		if err != nil {
			log.Fatal(err)
		}

	}
	tx.Commit()
	return
}

type user2CourseInfo struct {
	courseID  int64
	classID   int64
	useID     string
	role      int
	typeDicFk int64
}

func insert2course2User(database *sql.DB) (err error) {
	rows, err := database.Query("select account,uid from user_info")
	if err != nil {
		log.Fatal(err)
	}
	defer rows.Close()
	// var uidList map[string]string
	uidList := make(map[string]string)
	for rows.Next() {
		var uid string
		var account string
		err = rows.Scan(&account, &uid)
		if err != nil {
			log.Fatal(err)
		} else {
			uidList[account] = uid
		}
	}
	err = rows.Err()
	if err != nil {
		log.Fatal(err)
	}

	rows, err = database.Query("select courseId,classId,typeDicFk from course_info GROUP BY courseId")
	if err != nil {
		log.Fatal(err)
	}
	defer rows.Close()

	var u2courseList []user2CourseInfo
	for rows.Next() {
		var r2cInfo user2CourseInfo
		err = rows.Scan(&r2cInfo.courseID, &r2cInfo.classID, &r2cInfo.typeDicFk)
		if err != nil {
			log.Fatal(err)
			continue
		}
		u2courseList = append(u2courseList, r2cInfo)
	}
	err = rows.Err()
	if err != nil {
		log.Fatal(err)
	}

	tx, err := database.Begin()
	if err != nil {
		log.Fatal(err)
	}
	stmt, err := tx.Prepare("INSERT INTO user2course_info VALUES (?,?,?,?,?);")
	if err != nil {
		log.Fatal(err)
	}
	defer stmt.Close()

	for account, uid := range uidList {

		for _, u2cinfo := range u2courseList {
			role := 0
			if strings.Contains(account, "stu") {
				role = 0
			} else if strings.Contains(account, "tea3000") {
				role = 3000
			} else if strings.Contains(account, "tea5702") {
				role = 5702
			} else if strings.Contains(account, "tea5703") {
				role = 5703
			} else if strings.Contains(account, "tea5701") {
				role = 5701
			}
			classid := u2cinfo.classID
			r := mathRand.New(mathRand.NewSource(time.Now().UnixNano()))
			count := r.Intn(100)
			if 0 == role && count < 50 { //随机不分班30% 0表示未分班
				classid = 0
				time.Sleep(2 * time.Second)
			}
			if _, err = stmt.Exec(uid, u2cinfo.courseID, classid, false, role); err != nil {
				log.Fatal(err)
				return
			}
		}
	}
	tx.Commit()
	return
}

func createTable(database *sql.DB) (err1 error) {

	fucnCreateSQL := func(strSql string) (err1 error) {
		_, err1 = database.Exec(strSql)
		if err1 != nil {
			fmt.Println(strSql)
			fmt.Println(err1.Error())
		}
		return
	}
	if err1 = fucnCreateSQL(sqlCreateUserTable); err1 != nil {
		return
	}
	if err1 = fucnCreateSQL(sqlCreateCourseTable); err1 != nil {
		return
	}
	return fucnCreateSQL(sqlCreateUser2CourseTable)
}

var sqlCreateUserTable = `
create table if not exists user_info (
	account 	VARCHAR(64) not null UNIQUE,
	password	VARCHAR(64) not null,
	uid			INTEGER		not null UNIQUE,
	qq			VARCHAR(32) DEFAULT "313245022",
	nickname	VARCHAR(64) DEFAULT "defaulte my Name",
	headImg		VARCHAR(128),
	email		VARCHAR(64)	DEFAULT "1381314222@qq.com",
	phone		VARCHAR(64)	DEFAULT "1381314222",
	token		VARCHAR(64)	not null UNIQUE,
	role		INTEGER		DEFAULT 0
	);
`
var sqlCreateCourseTable = `
create table if not exists course_info (
	courseId	integer 	not null,
	classId 	integer		not null,
	chapterId   integer		not null,
	manageMember TEXT,
	isLiving	BOOLEAN DEFAULT false,
	mainTeacher VARCHAR(64) DEFAULT "abcdef",
	title		VARCHAR(64) DEFAULT "",
	chaptertile VARCHAR(64) DEFAULT "",
	cover		VARCHAR(64) DEFAULT "",
	chapterTime integer DEFAULT 0,
	announcement VARCHAR(255),
	qqlist		VARCHAR(255),
	typeDicFk	INTEGER DEFAULT 5311
);
CREATE INDEX "courseId" ON "course_info" ("courseId");
CREATE INDEX "classId" ON "course_info" ("classId");
CREATE INDEX "chapterId" ON "course_info" ("chapterId");
`
var sqlCreateUser2CourseTable = `
create table if not exists user2course_info (
	useId 		integer 	not null,
	courseId	integer 	not null,
	classId 	integer		,
	collectCourse BOOLEAN DEFAULT false,
	role		integer		DEFAULT 0
);
CREATE INDEX  "useId" ON "user2course_info" ("useId","courseId","classId");
`

//生成32位md5字串
func GetMd5String(s string) string {
	h := md5.New()
	h.Write([]byte(s))
	return hex.EncodeToString(h.Sum(nil))
}

//生成Guid字串
func GetGUID() string {
	b := make([]byte, 48)

	if _, err := io.ReadFull(rand.Reader, b); err != nil {
		return ""
	}
	return GetMd5String(base64.URLEncoding.EncodeToString(b))
}

// func getStrInsert2User(id, uid, role int64, account, qq, nickname, headImg, email, phone, token string) string {
// 	fun2Str := func(id int64) string {
// 		return strconv.FormatInt(id, 10)
// 	}

// 	strInsert2User := `INSERT INTO user_info VALUES (%1,"%2",%3,"%4","%5","%6","%7","%8","%9",%10);`
// 	rsp := strings.NewReplacer("%1", fun2Str(id), "%2", account, "%3", fun2Str(uid), "%4", qq, "%5", nickname,
// 		"%6", headImg, "%7", email, "%8", phone, "%9", token, "%10", fun2Str(role))
// 	return rsp.Replace(strInsert2User)
// }
// func getStrInsert2Course(courseID, classID, chapterID, typeDicFk int64,
// 	title, cover, chapterTime, announcement, qqlist string, CollectCourse bool) string {
// 	fun2Str := func(id int64) string {
// 		return strconv.FormatInt(id, 10)
// 	}
// 	strInsert2Course := `INSERT INTO course_info VALUES (%1,%2,%3,"%4","%5","%6","%7","%8",%9,%10);`
// 	rsp := strings.NewReplacer("%1", fun2Str(courseID), "%2", fun2Str(classID),
// 		"%3", fun2Str(chapterID), "%4", title, "%5", cover, "%6", chapterTime, "%7", announcement,
// 		"%8", qqlist, "%9", "0", "%10", fun2Str(typeDicFk))

// 	return rsp.Replace(strInsert2Course)
// }
// func getStrInsert2User2Course(useID, courseID, classID int64, role int) string {
// 	fun2Str := func(id int64) string {
// 		return strconv.FormatInt(id, 10)
// 	}
// 	strInsert2User2Course := `INSERT INTO user2course_info VALUES (%1,%2,%3,%4);`
// 	rsp := strings.NewReplacer("%1", fun2Str(useID), "%2", fun2Str(courseID),
// 		"%3", fun2Str(classID), "%4", strconv.Itoa(role))
// 	return rsp.Replace(strInsert2User2Course)
// }
