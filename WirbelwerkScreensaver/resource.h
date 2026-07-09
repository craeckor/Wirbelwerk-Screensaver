#pragma once

// ==================== DIALOGS ====================
#define IDD_CONFIGDIALOG                101

// ==================== STATIC (NON-INTERACTIVE) LABELS/GROUPBOXES ====================
// Standard "no lookup needed" placeholder id, matches the convention used by the
// Visual Studio dialog editor (IDC_STATIC == -1). Safe to repeat many times in one dialog.
#define IDC_STATIC                      -1

// ==================== COLOR GROUP ====================
#define IDC_COLOR_RAINBOW               1000
#define IDC_COLOR_SINGLE                1001
#define IDC_COLOR_RANGE                 1002

#define IDC_EDIT_COLOR_SINGLE           1010
#define IDC_BTN_PICK_SINGLE             1011
#define IDC_SWATCH_SINGLE               1012

#define IDC_EDIT_COLOR_RANGE_START      1020
#define IDC_BTN_PICK_RANGE_START        1021
#define IDC_SWATCH_RANGE_START          1022

#define IDC_EDIT_COLOR_RANGE_END        1030
#define IDC_BTN_PICK_RANGE_END          1031
#define IDC_SWATCH_RANGE_END            1032

#define IDC_CHECK_SMOOTH_SPAWN          1040
#define IDC_SLIDER_SMOOTH_MS            1041
#define IDC_LABEL_SMOOTH_MS             1042

// ==================== FLOW GROUP ====================
#define IDC_SLIDER_CURL                 1100
#define IDC_LABEL_CURL                  1101
#define IDC_SLIDER_VELDISS              1102
#define IDC_LABEL_VELDISS               1103
#define IDC_SLIDER_DENDISS              1104
#define IDC_LABEL_DENDISS               1105
#define IDC_SLIDER_PRESSURE             1106
#define IDC_LABEL_PRESSURE              1107
#define IDC_SLIDER_PRESSITER            1108
#define IDC_LABEL_PRESSITER             1109

// ==================== BRUSH GROUP ====================
#define IDC_SLIDER_SPLATRADIUS          1120
#define IDC_LABEL_SPLATRADIUS           1121
#define IDC_SLIDER_SPLATFORCE           1122
#define IDC_LABEL_SPLATFORCE            1123
#define IDC_SLIDER_DYEAMOUNT            1124
#define IDC_LABEL_DYEAMOUNT             1125

// ==================== QUALITY GROUP ====================
#define IDC_COMBO_SIMRES                1140
#define IDC_COMBO_DYERES                1141
#define IDC_CHECK_SHADING               1142

// ==================== BEHAVIOR GROUP ====================
#define IDC_CHECK_AUTOBURST             1150
#define IDC_SLIDER_AUTOINTERVAL         1151
#define IDC_LABEL_AUTOINTERVAL          1152
#define IDC_SLIDER_FPSLIMIT             1153
#define IDC_LABEL_FPSLIMIT              1154
#define IDC_CHECK_PERMONITOR            1155

// ==================== FOOTER ====================
#define IDC_BTN_PREVIEW                 1170
#define IDC_BTN_DEFAULTS                1171
#define IDC_LABEL_CONFIGPATH            1172

// ==================== LANGUAGE ====================
#define IDC_COMBO_LANGUAGE              1180

// ==================== GROUPBOXES ====================
#define IDC_GROUP_COLOR                 1190
#define IDC_GROUP_FLOW                  1191
#define IDC_GROUP_BRUSH                 1192
#define IDC_GROUP_QUALITY               1193
#define IDC_GROUP_BEHAVIOR              1194

// ==================== INFO BUTTONS ====================
#define IDC_INFO_CURL                   1200
#define IDC_INFO_VELDISS                1201
#define IDC_INFO_DENDISS                1202
#define IDC_INFO_PRESSURE               1203
#define IDC_INFO_PRESSITER              1204
#define IDC_INFO_SPLATRADIUS            1205
#define IDC_INFO_SPLATFORCE             1206
#define IDC_INFO_DYEAMOUNT              1207
#define IDC_INFO_SIMRES                 1208
#define IDC_INFO_DYERES                 1209
#define IDC_INFO_SHADING                1210
#define IDC_INFO_SMOOTHSPAWN            1211
#define IDC_INFO_AUTOBURST              1212
#define IDC_INFO_FPSLIMIT               1213
#define IDC_INFO_PERMONITOR             1214
#define IDC_INFO_COLORMODE              1215

// ==================== MANUAL EDIT FIELDS (numeric overrides) ====================
#define IDC_EDIT_CURL                   1220
#define IDC_EDIT_VELDISS                1221
#define IDC_EDIT_DENDISS                1222
#define IDC_EDIT_PRESSURE               1223
#define IDC_EDIT_PRESSITER              1224
#define IDC_EDIT_SPLATRADIUS            1225
#define IDC_EDIT_SPLATFORCE             1226
#define IDC_EDIT_DYEAMOUNT              1227
#define IDC_EDIT_SMOOTH_MS              1228
#define IDC_EDIT_AUTOINTERVAL           1229
#define IDC_EDIT_FPSLIMIT               1230

// ==================== DESCRIPTIVE STATIC LABELS (unique IDs for translation) ====================
#define IDC_LABEL_LANGUAGE              1240
#define IDC_LABEL_COLOR_SINGLE_DESC     1241
#define IDC_LABEL_RANGE_START_DESC      1242
#define IDC_LABEL_RANGE_END_DESC        1243
#define IDC_LABEL_CURL_DESC             1244
#define IDC_LABEL_VELDISS_DESC          1245
#define IDC_LABEL_DENDISS_DESC          1246
#define IDC_LABEL_PRESSURE_DESC         1247
#define IDC_LABEL_PRESSITER_DESC        1248
#define IDC_LABEL_SPLATRADIUS_DESC      1249
#define IDC_LABEL_SPLATFORCE_DESC       1250
#define IDC_LABEL_DYEAMOUNT_DESC        1251
#define IDC_LABEL_SIMRES_DESC           1252
#define IDC_LABEL_DYERES_DESC           1253
#define IDC_LABEL_FPSLIMIT_DESC         1254
