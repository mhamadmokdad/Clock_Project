
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;
// ثوابت النافذة
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char* WINDOW_TITLE = "My Clock Project";

// متغيرات الوقت
float currentTime = 0.0f;
float lastTime = 0.0f;
int seconds = 0;
int minutes = 30;   // نبدأ عند 3:30 مثلاً
int hours = 3;

// دالة قراءة ملف الشيدر (نص بسيط)
string readShader(const char* filename) {
        ifstream file(filename);
    if (!file.is_open()) {
        cerr << "خطأ: لم أستطع فتح ملف " << filename << endl;
        return "";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// دالة فحص أخطاء الشيدر (أخذتها من النت لكني فهمتها)
void checkShader(GLuint shader, const char* type) {
    GLint success;
    GLchar log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);
        cout << "خطأ في ترجمة " << type << ":\n" << log << endl;
    }
}

// دالة رسم دائرة (باستخدام GL_TRIANGLE_FAN)
vector<float> makeCircle(float cx, float cy, float r, int segments, float R, float G, float B) {
    vector<float> verts;
    // نقطة المركز (للمثلث fan)
    verts.push_back(cx);
    verts.push_back(cy);
    verts.push_back(0.0f);
    verts.push_back(R);
    verts.push_back(G);
    verts.push_back(B);

    // نقاط المحيط
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = cx + r * cos(angle);
        float y = cy + r * sin(angle);
        verts.push_back(x);
        verts.push_back(y);
        verts.push_back(0.0f);
        verts.push_back(R);
        verts.push_back(G);
        verts.push_back(B);
    }
    return verts;
}

// دالة رسم خط (نقطتان)
vector<float> makeLine(float x1, float y1, float x2, float y2, float R, float G, float B) {
    vector<float> verts;
    // نقطة أولى
    verts.push_back(x1); verts.push_back(y1); verts.push_back(0.0f);
    verts.push_back(R); verts.push_back(G); verts.push_back(B);
    // نقطة ثانية
    verts.push_back(x2); verts.push_back(y2); verts.push_back(0.0f);
    verts.push_back(R); verts.push_back(G); verts.push_back(B);
    return verts;
}

// تحديث الوقت (كل ثانية)
void updateTime() {
    currentTime = glfwGetTime();
    float delta = currentTime - lastTime;
    if (delta >= 1.0f) {
        seconds++;
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
            if (minutes >= 60) {
                minutes = 0;
                hours++;
                if (hours >= 12) hours = 0;
            }
        }
        lastTime = currentTime;

    }
}

// زوايا العقارب (بالراديان)
float getSecondAngle() {
    return 3.14159f / 2 - (seconds / 60.0f) * 2.0f * 3.14159f; // نبدأ من الأعلى (12)
}

float getMinuteAngle() {
    float angle = 3.14159f / 2 - (minutes / 60.0f) * 2.0f * 3.14159f;
    // إضافة حركة خفيفة من الثواني (اختياري)
    angle -= (seconds / 60.0f) * (2.0f * 3.14159f / 60.0f);
    return angle;
}

float getHourAngle() {
    float angle = 3.14159f / 2 - ((hours % 12) / 12.0f) * 2.0f * 3.14159f;
    angle -= (minutes / 60.0f) * (2.0f * 3.14159f / 12.0f);
    return angle;
}

int main() {
    // تهيئة GLFW
    if (!glfwInit()) {
        cerr << "فشل في تهيئة GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!window) {
        cerr << "فشل في إنشاء النافذة" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cerr << "فشل في تهيئة GLEW" << endl;
        return -1;
    }

    // قراءة الشيدرات
    string vertexSrc = readShader("vertex_shader.glsl");
    string fragmentSrc = readShader("fragment_shader.glsl");
    const char* vSrc = vertexSrc.c_str();
    const char* fSrc = fragmentSrc.c_str();

    // بناء vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vSrc, NULL);
    glCompileShader(vertexShader);
    checkShader(vertexShader, "vertex shader");

    // بناء fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fSrc, NULL);
    glCompileShader(fragmentShader);
    checkShader(fragmentShader, "fragment shader");

    // برنامج الشيدر
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // حذف الشيدرات بعد الربط (لم نعد بحاجة لها)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

 
    // بناء بيانات الساعة (جميع الرؤوس في مصفوفة واحدة)

    vector<float> vertices;
    vector<int> startIndices;    // مؤشر بداية كل جزء (عدد الرؤوس)
    vector<int> countIndices;    // عدد رؤوس كل جزء
    vector<GLenum> modes;        // نوع الرسم لكل جزء

    // 1. وجه الساعة (دائرة كبيرة)
    vector<float> face = makeCircle(0.0f, 0.0f, 0.8f, 64, 0.95f, 0.95f, 0.95f);
    startIndices.push_back(vertices.size() / 6); // كل رأس = 6 floats
    countIndices.push_back(face.size() / 6);
    modes.push_back(GL_TRIANGLE_FAN);
    vertices.insert(vertices.end(), face.begin(), face.end());

    // 2. علامات الساعات (12 دائرة صغيرة)
    for (int i = 0; i < 12; i++) {
        float angle = (i / 12.0f) * 2.0f * 3.14159f - 3.14159f / 2;
        float x = 0.7f * cos(angle);
        float y = 0.7f * sin(angle);
        vector<float> mark = makeCircle(x, y, 0.05f, 8, 0.2f, 0.2f, 0.2f);
        startIndices.push_back(vertices.size() / 6);
        countIndices.push_back(mark.size() / 6);
        modes.push_back(GL_TRIANGLE_FAN);
        vertices.insert(vertices.end(), mark.begin(), mark.end());
    }

    // 3. مركز الساعة (دائرة صغيرة حمراء)
    vector<float> center = makeCircle(0.0f, 0.0f, 0.03f, 8, 1.0f, 0.0f, 0.0f);
    startIndices.push_back(vertices.size() / 6);
    countIndices.push_back(center.size() / 6);
    modes.push_back(GL_TRIANGLE_FAN);
    vertices.insert(vertices.end(), center.begin(), center.end());

    // 4. عقرب الساعات (سنقوم بتحديثه كل إطار، نضع بيانات مؤقتة)
    vector<float> hourHand = makeLine(0, 0, 0.35f, 0, 0.0f, 0.0f, 1.0f);
    startIndices.push_back(vertices.size() / 6);
    countIndices.push_back(2); // خط = رأسين
    modes.push_back(GL_LINES);
    vertices.insert(vertices.end(), hourHand.begin(), hourHand.end());

    // 5. عقرب الدقائق
    vector<float> minuteHand = makeLine(0, 0, 0.5f, 0, 0.0f, 1.0f, 0.0f);
    startIndices.push_back(vertices.size() / 6);
    countIndices.push_back(2);
    modes.push_back(GL_LINES);
    vertices.insert(vertices.end(), minuteHand.begin(), minuteHand.end());

    // 6. عقرب الثواني
    vector<float> secondHand = makeLine(0, 0, 0.6f, 0, 1.0f, 0.0f, 0.0f);
    startIndices.push_back(vertices.size() / 6);
    countIndices.push_back(2);
    modes.push_back(GL_LINES);
    vertices.insert(vertices.end(), secondHand.begin(), secondHand.end());

    // إنشاء VAO و VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    // الموقع (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // اللون (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // مكان متغير الوقت في الـ shader
    GLint timeUniform = glGetUniformLocation(shaderProgram, "u_time");

    // حلقة الرسم
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        updateTime();

        // حساب الزوايا الحالية
        float secA = getSecondAngle();
        float minA = getMinuteAngle();
        float hourA = getHourAngle();

        // حساب نقاط نهاية العقارب
        float secX = 0.6f * cos(secA);
        float secY = 0.6f * sin(secA);
        float minX = 0.5f * cos(minA);
        float minY = 0.5f * sin(minA);
        float hourX = 0.35f * cos(hourA);
        float hourY = 0.35f * sin(hourA);

        // تحديث بيانات العقارب في الـ VBO
        // نحدد أماكنها في المصفوفة
        int offsetSecond = (startIndices[14] * 6 * sizeof(float)); // بعد 14 قطعة
        vector<float> newSecond = {
            0, 0, 0, 1, 0, 0,
            secX, secY, 0, 1, 0, 0
        };
        glBufferSubData(GL_ARRAY_BUFFER, offsetSecond, newSecond.size() * sizeof(float), newSecond.data());

        int offsetMinute = (startIndices[15] * 6 * sizeof(float));
        vector<float> newMinute = {
            0, 0, 0, 0, 1, 0,
            minX, minY, 0, 0, 1, 0
        };
        glBufferSubData(GL_ARRAY_BUFFER, offsetMinute, newMinute.size() * sizeof(float), newMinute.data());

        int offsetHour = (startIndices[16] * 6 * sizeof(float));
        vector<float> newHour = {
            0, 0, 0, 0, 0, 1,
            hourX, hourY, 0, 0, 0, 1
        };
        glBufferSubData(GL_ARRAY_BUFFER, offsetHour, newHour.size() * sizeof(float), newHour.data());

        // تنظيف الشاشة
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform1f(timeUniform, glfwGetTime()); // يمكن استخدامه في الشيدر لاحقاً

        glBindVertexArray(VAO);

        // رسم جميع العناصر حسب الأنواع المخزنة
        for (size_t i = 0; i < startIndices.size(); i++) {
            glDrawArrays(modes[i], startIndices[i], countIndices[i]);
        }

        glfwSwapBuffers(window);
    }

    // تنظيف
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}