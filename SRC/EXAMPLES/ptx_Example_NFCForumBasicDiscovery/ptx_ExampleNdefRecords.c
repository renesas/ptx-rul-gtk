/*
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2026, Renesas Electronics Corporation and/or its affiliates


    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of
       conditions and the following disclaimer in the documentation and/or other
       materials provided with the distribution.

    3. Neither the name of Renesas nor the names of its
       contributors may be used to endorse or promote products derived from this
       software without specific prior written permission.



    THIS SOFTWARE IS PROVIDED BY Renesas "AS IS" AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL RENESAS OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    ---------------------------------------------------------------


    Project     : PTX2xxR/W
    Module      : SDK Example (NFC Forum Basic Discovery)
    File        : EXAMPLES/ptx_Example_NFCForumBasicDiscovery/ptx_ExampleNdefRecords.c

    Description : Table-driven definitions and encoders of standard NDEF record types.
                  Payloads are assembled here (bounds-checked), records and messages are created
                  using ptx_NDEFRecord_Create and ptx_NDEFMessage_Create of the SDK.
*/

#include "ptx_ExampleNdefRecords.h"
#include "ptx_Status.h"
#include "ptx_NDEFRecord.h"
#include "ptx_NDEFMessage.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / CONSTANTS
 * ####################################################################################################################
 */

/* Max. size of a nested NDEF message (Smart Poster payload). */
#define NESTED_MESSAGE_MAX_SIZE     256u

/* Max. length of a summary text before it gets shortened. */
#define SUMMARY_TEXT_MAX_CHARS      40

/* Max. length of the language code of a Text record (6 bits in the status byte). */
#define TEXT_LANGUAGE_MAX_LEN       63u

/* URI identifier codes (NFC Forum URI RTD), indexed by the first payload byte of a URI record. */
static const char * const URI_PREFIXES[] = {
    "", "http://www.", "https://www.", "http://", "https://", "tel:", "mailto:",
    "ftp://anonymous:anonymous@", "ftp://ftp.", "ftps://", "sftp://", "smb://", "nfs://",
    "ftp://", "dav://", "news:", "telnet://", "imap:", "rtsp://", "urn:", "pop:", "sip:",
    "sips:", "tftp:", "btspp://", "btl2cap://", "btgoep://", "tcpobex://", "irdaobex://",
    "file://", "urn:epc:id:", "urn:epc:tag:", "urn:epc:pat:", "urn:epc:raw:", "urn:epc:", "urn:nfc:"
};

/* Wi-Fi Simple Configuration (WSC) attribute types, used by the Wi-Fi record. */
#define WSC_ATTR_VERSION            0x104Au
#define WSC_ATTR_CREDENTIAL         0x100Eu
#define WSC_ATTR_NETWORK_INDEX      0x1026u
#define WSC_ATTR_SSID               0x1045u
#define WSC_ATTR_AUTH_TYPE          0x1003u
#define WSC_ATTR_ENCRYPTION_TYPE    0x100Fu
#define WSC_ATTR_NETWORK_KEY        0x1027u
#define WSC_ATTR_MAC_ADDRESS        0x1020u

#define WSC_VERSION_1_0             0x10u
#define WIFI_SSID_MAX_LEN           32u
#define WIFI_KEY_MIN_LEN            8u
#define WIFI_KEY_MAX_LEN            63u

/* Wi-Fi authentication choices and their WSC authentication / encryption types. */
static const char * const WIFI_AUTH_CHOICES[] = { "WPA2-Personal", "WPA-Personal", "Open", NULL };
static const uint16_t WIFI_AUTH_TYPES[]       = { 0x0020u,         0x0002u,        0x0001u };
static const uint16_t WIFI_ENCRYPTION_TYPES[] = { 0x0008u,         0x0004u,        0x0001u };   /* AES, TKIP, None */

/*
 * ####################################################################################################################
 * HELPERS
 * ####################################################################################################################
 */

static GQuark ndefErrorQuark(void)
{
    return g_quark_from_static_string("ptx-example-ndef-error");
}

static gboolean isBlank(const gchar *s)
{
    if (NULL == s)
        return TRUE;

    for (; '\0' != *s; ++s) {
        if (!g_ascii_isspace(*s))
            return FALSE;
    }
    return TRUE;
}

/* Returns a newly allocated copy without leading/trailing whitespace. */
static gchar *stripped(const gchar *s)
{
    return g_strstrip(g_strdup((NULL != s) ? s : ""));
}

/* Returns a newly allocated copy without any whitespace (phone numbers). */
static gchar *withoutSpaces(const gchar *s)
{
    GString *out = g_string_new(NULL);
    for (; (NULL != s) && ('\0' != *s); ++s) {
        if (!g_ascii_isspace(*s))
            g_string_append_c(out, *s);
    }
    return g_string_free(out, FALSE);
}

/* Returns a newly allocated, shortened copy for summaries. */
static gchar *shortText(const gchar *s)
{
    if (g_utf8_strlen(s, -1) <= SUMMARY_TEXT_MAX_CHARS)
        return g_strdup(s);

    gchar *head = g_utf8_substring(s, 0, SUMMARY_TEXT_MAX_CHARS);
    gchar *result = g_strconcat(head, "...", NULL);
    g_free(head);
    return result;
}

static void appendString(GByteArray *array, const char *s)
{
    g_byte_array_append(array, (const guint8 *)s, (guint)strlen(s));
}

static void setType(NdefRecordData *out, uint8_t tnf, const char *type)
{
    out->tnf = tnf;
    g_byte_array_set_size(out->type, 0);
    appendString(out->type, type);
}

/* Appends a WSC attribute (type and length are 2 bytes each, big-endian). */
static void appendTlv(GByteArray *array, uint16_t type, const guint8 *data, uint16_t len)
{
    const guint8 header[4] = { (guint8)(type >> 8), (guint8)type, (guint8)(len >> 8), (guint8)len };
    g_byte_array_append(array, header, sizeof(header));
    if (len > 0u)
        g_byte_array_append(array, data, len);
}

static void appendTlvU16(GByteArray *array, uint16_t type, uint16_t value)
{
    const guint8 data[2] = { (guint8)(value >> 8), (guint8)value };
    appendTlv(array, type, data, sizeof(data));
}

static NdefRecordData *recordDataNew(void)
{
    NdefRecordData *data = g_new0(NdefRecordData, 1);
    data->type = g_byte_array_new();
    data->payload = g_byte_array_new();
    return data;
}

/* URI record payload: identifier code of the longest matching prefix, followed by the remaining URI. */
static void encodeUriRecord(NdefRecordData *out, const char *uri)
{
    uint8_t code = 0;
    size_t prefixLen = 0;

    for (uint8_t i = 1; i < G_N_ELEMENTS(URI_PREFIXES); ++i) {
        size_t len = strlen(URI_PREFIXES[i]);
        if ((len > prefixLen) && g_str_has_prefix(uri, URI_PREFIXES[i])) {
            code = i;
            prefixLen = len;
        }
    }

    setType(out, TNF_WELL_KNOWN_TYPE, "U");
    g_byte_array_set_size(out->payload, 0);
    g_byte_array_append(out->payload, &code, 1);
    appendString(out->payload, &uri[prefixLen]);
}

/* Text record payload: status byte (UTF-8, language code length), language code, text. */
static gboolean encodeTextRecord(NdefRecordData *out, const char *language, const char *text, GError **error)
{
    size_t langLen = strlen(language);

    if ((0u == langLen) || (langLen > TEXT_LANGUAGE_MAX_LEN)) {
        g_set_error(error, ndefErrorQuark(), 0, "Language code must have 1 to %u characters (e.g. \"en\").", TEXT_LANGUAGE_MAX_LEN);
        return FALSE;
    }

    const guint8 status = (guint8)langLen;
    setType(out, TNF_WELL_KNOWN_TYPE, "T");
    g_byte_array_set_size(out->payload, 0);
    g_byte_array_append(out->payload, &status, 1);
    appendString(out->payload, language);
    appendString(out->payload, text);
    return TRUE;
}

static gboolean parseCoordinate(const gchar *s, double limit, const char *name, GError **error)
{
    gchar *end = NULL;
    double value = g_ascii_strtod(s, &end);

    /* NaN fails the range check as well */
    if ((end == s) || ('\0' != *end) || !(value >= -limit) || !(value <= limit)) {
        g_set_error(error, ndefErrorQuark(), 0, "%s must be a number between %.0f and %.0f (use '.' as decimal separator).", name, -limit, limit);
        return FALSE;
    }
    return TRUE;
}

/* Escapes a vCard 3.0 text value. */
static void appendVCardValue(GString *card, const gchar *value)
{
    for (; '\0' != *value; ++value) {
        switch (*value) {
            case '\\': g_string_append(card, "\\\\"); break;
            case ',':  g_string_append(card, "\\,"); break;
            case ';':  g_string_append(card, "\\;"); break;
            case '\n': g_string_append(card, "\\n"); break;
            case '\r': break;
            default:   g_string_append_c(card, *value); break;
        }
    }
}

static void appendVCardProperty(GString *card, const char *name, const gchar *value)
{
    if (isBlank(value))
        return;

    gchar *v = stripped(value);
    g_string_append_printf(card, "%s:", name);
    appendVCardValue(card, v);
    g_string_append(card, "\r\n");
    g_free(v);
}

/*
 * ####################################################################################################################
 * RECORD TYPE ENCODERS
 * The field values are passed in the order of the field definitions. Required fields are checked to be non-blank
 * before an encoder is called.
 * ####################################################################################################################
 */

static gboolean encodeUri(const gchar * const *values, NdefRecordData *out, GError **error)
{
    (void)error;
    gchar *uri = stripped(values[0]);
    encodeUriRecord(out, uri);
    out->summary = g_strdup_printf("URI: %s", uri);
    g_free(uri);
    return TRUE;
}

static gboolean encodeText(const gchar * const *values, NdefRecordData *out, GError **error)
{
    gchar *language = stripped(values[0]);
    gboolean ok = encodeTextRecord(out, language, values[1], error);
    if (ok) {
        gchar *text = shortText(values[1]);
        out->summary = g_strdup_printf("Text (%s): %s", language, text);
        g_free(text);
    }
    g_free(language);
    return ok;
}

static gboolean encodeSmartPoster(const gchar * const *values, NdefRecordData *out, GError **error)
{
    gchar *uri = stripped(values[0]);
    gchar *language = isBlank(values[2]) ? g_strdup("en") : stripped(values[2]);
    GPtrArray *nested = g_ptr_array_new_with_free_func((GDestroyNotify)ptx_ExampleNdef_RecordDataFree);
    gboolean ok = TRUE;

    NdefRecordData *uriRecord = recordDataNew();
    encodeUriRecord(uriRecord, uri);
    g_ptr_array_add(nested, uriRecord);

    if (!isBlank(values[1])) {
        NdefRecordData *titleRecord = recordDataNew();
        g_ptr_array_add(nested, titleRecord);
        ok = encodeTextRecord(titleRecord, language, values[1], error);
    }

    if (ok) {
        guint8 message[NESTED_MESSAGE_MAX_SIZE];
        uint32_t len = (uint32_t)sizeof(message);
        ok = ptx_ExampleNdef_BuildMessage(nested, message, &len, error);
        if (ok) {
            setType(out, TNF_WELL_KNOWN_TYPE, "Sp");
            g_byte_array_append(out->payload, message, len);
            out->summary = isBlank(values[1]) ? g_strdup_printf("Smart Poster: %s", uri)
                                              : g_strdup_printf("Smart Poster: %s (%s)", uri, values[1]);
        }
    }

    g_ptr_array_unref(nested);
    g_free(language);
    g_free(uri);
    return ok;
}

static gboolean encodePhone(const gchar * const *values, NdefRecordData *out, GError **error)
{
    (void)error;
    gchar *number = withoutSpaces(values[0]);
    gchar *uri = g_strconcat("tel:", number, NULL);
    encodeUriRecord(out, uri);
    out->summary = g_strdup_printf("Phone: %s", number);
    g_free(uri);
    g_free(number);
    return TRUE;
}

static gboolean encodeEmail(const gchar * const *values, NdefRecordData *out, GError **error)
{
    (void)error;
    gchar *address = stripped(values[0]);
    GString *uri = g_string_new("mailto:");
    char separator = '?';

    g_string_append(uri, address);
    if (!isBlank(values[1])) {
        gchar *subject = g_uri_escape_string(values[1], NULL, FALSE);
        g_string_append_printf(uri, "%csubject=%s", separator, subject);
        separator = '&';
        g_free(subject);
    }
    if (!isBlank(values[2])) {
        gchar *body = g_uri_escape_string(values[2], NULL, FALSE);
        g_string_append_printf(uri, "%cbody=%s", separator, body);
        g_free(body);
    }

    encodeUriRecord(out, uri->str);
    out->summary = g_strdup_printf("Email: %s", address);
    g_string_free(uri, TRUE);
    g_free(address);
    return TRUE;
}

static gboolean encodeSms(const gchar * const *values, NdefRecordData *out, GError **error)
{
    (void)error;
    gchar *number = withoutSpaces(values[0]);
    GString *uri = g_string_new("sms:");

    g_string_append(uri, number);
    if (!isBlank(values[1])) {
        gchar *body = g_uri_escape_string(values[1], NULL, FALSE);
        g_string_append_printf(uri, "?body=%s", body);
        g_free(body);
    }

    encodeUriRecord(out, uri->str);
    out->summary = g_strdup_printf("SMS: %s", number);
    g_string_free(uri, TRUE);
    g_free(number);
    return TRUE;
}

static gboolean encodeGeo(const gchar * const *values, NdefRecordData *out, GError **error)
{
    gchar *latitude = stripped(values[0]);
    gchar *longitude = stripped(values[1]);
    gboolean ok = parseCoordinate(latitude, 90.0, "Latitude", error) &&
                  parseCoordinate(longitude, 180.0, "Longitude", error);

    if (ok) {
        gchar *uri = g_strdup_printf("geo:%s,%s", latitude, longitude);
        encodeUriRecord(out, uri);
        out->summary = g_strdup_printf("Geo location: %s, %s", latitude, longitude);
        g_free(uri);
    }

    g_free(longitude);
    g_free(latitude);
    return ok;
}

static gboolean encodeVCard(const gchar * const *values, NdefRecordData *out, GError **error)
{
    (void)error;
    gchar *name = stripped(values[0]);
    GString *card = g_string_new("BEGIN:VCARD\r\nVERSION:3.0\r\n");

    g_string_append(card, "N:");
    appendVCardValue(card, name);
    g_string_append(card, ";;;;\r\n");
    appendVCardProperty(card, "FN", name);
    appendVCardProperty(card, "TEL", values[1]);
    appendVCardProperty(card, "EMAIL", values[2]);
    appendVCardProperty(card, "ORG", values[3]);
    g_string_append(card, "END:VCARD\r\n");

    setType(out, TNF_MEDIA_TYPE, "text/vcard");
    appendString(out->payload, card->str);
    out->summary = g_strdup_printf("vCard: %s", name);

    g_string_free(card, TRUE);
    g_free(name);
    return TRUE;
}

static gboolean encodeMime(const gchar * const *values, NdefRecordData *out, GError **error)
{
    gchar *mimeType = stripped(values[0]);
    const char *slash = strchr(mimeType, '/');
    gboolean ok = (NULL != slash) && (slash != mimeType) && ('\0' != slash[1]) && (strlen(mimeType) <= 255u);

    for (const char *c = mimeType; ok && ('\0' != *c); ++c) {
        ok = g_ascii_isgraph(*c);
    }

    if (ok) {
        setType(out, TNF_MEDIA_TYPE, mimeType);
        appendString(out->payload, values[1]);
        gchar *content = shortText(values[1]);
        out->summary = g_strdup_printf("MIME (%s): %s", mimeType, content);
        g_free(content);
    } else {
        g_set_error_literal(error, ndefErrorQuark(), 0, "MIME type must look like \"type/subtype\" (e.g. \"text/plain\").");
    }

    g_free(mimeType);
    return ok;
}

static gboolean encodeWifi(const gchar * const *values, NdefRecordData *out, GError **error)
{
    const gchar *ssid = values[0];
    const gchar *key = (NULL != values[2]) ? values[2] : "";
    size_t ssidLen = strlen(ssid);
    size_t keyLen = strlen(key);
    guint auth = 0;

    while ((NULL != WIFI_AUTH_CHOICES[auth]) && (0 != g_strcmp0(WIFI_AUTH_CHOICES[auth], values[1]))) {
        auth++;
    }
    gboolean isOpen = (0x0001u == WIFI_AUTH_TYPES[auth]);

    if (ssidLen > WIFI_SSID_MAX_LEN) {
        g_set_error(error, ndefErrorQuark(), 0, "SSID must not be longer than %u bytes.", WIFI_SSID_MAX_LEN);
        return FALSE;
    }
    if (isOpen) {
        keyLen = 0;
    } else if ((keyLen < WIFI_KEY_MIN_LEN) || (keyLen > WIFI_KEY_MAX_LEN)) {
        g_set_error(error, ndefErrorQuark(), 0, "Network key must have %u to %u characters.", WIFI_KEY_MIN_LEN, WIFI_KEY_MAX_LEN);
        return FALSE;
    }

    static const guint8 networkIndex = 0x01u;
    static const guint8 version = WSC_VERSION_1_0;
    static const guint8 broadcastMac[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    GByteArray *credential = g_byte_array_new();
    appendTlv(credential, WSC_ATTR_NETWORK_INDEX, &networkIndex, 1u);
    appendTlv(credential, WSC_ATTR_SSID, (const guint8 *)ssid, (uint16_t)ssidLen);
    appendTlvU16(credential, WSC_ATTR_AUTH_TYPE, WIFI_AUTH_TYPES[auth]);
    appendTlvU16(credential, WSC_ATTR_ENCRYPTION_TYPE, WIFI_ENCRYPTION_TYPES[auth]);
    appendTlv(credential, WSC_ATTR_NETWORK_KEY, (const guint8 *)key, (uint16_t)keyLen);
    appendTlv(credential, WSC_ATTR_MAC_ADDRESS, broadcastMac, sizeof(broadcastMac));

    setType(out, TNF_MEDIA_TYPE, "application/vnd.wfa.wsc");
    appendTlv(out->payload, WSC_ATTR_VERSION, &version, 1u);
    appendTlv(out->payload, WSC_ATTR_CREDENTIAL, credential->data, (uint16_t)credential->len);
    out->summary = g_strdup_printf("Wi-Fi: %s (%s)", ssid, WIFI_AUTH_CHOICES[auth]);

    g_byte_array_unref(credential);
    return TRUE;
}

static gboolean encodeAndroidApp(const gchar * const *values, NdefRecordData *out, GError **error)
{
    gchar *package = stripped(values[0]);
    gboolean ok = (NULL != strchr(package, '.'));

    for (const char *c = package; ok && ('\0' != *c); ++c) {
        ok = g_ascii_isalnum(*c) || ('.' == *c) || ('_' == *c);
    }

    if (ok) {
        setType(out, TNF_EXTERNAL_TYPE, "android.com:pkg");
        appendString(out->payload, package);
        out->summary = g_strdup_printf("Android app: %s", package);
    } else {
        g_set_error_literal(error, ndefErrorQuark(), 0, "Package name must look like \"com.example.app\".");
    }

    g_free(package);
    return ok;
}

/*
 * ####################################################################################################################
 * RECORD TYPE TABLE
 * ####################################################################################################################
 */

static const NdefFieldDef URI_FIELDS[] = {
    { "URI", NdefField_Text, TRUE, "https://www.renesas.com/en/products/ptx205r", NULL },
};

static const NdefFieldDef TEXT_FIELDS[] = {
    { "Language", NdefField_Text, TRUE, "en", NULL },
    { "Text", NdefField_Text, TRUE, "", NULL },
};

static const NdefFieldDef SMART_POSTER_FIELDS[] = {
    { "URI", NdefField_Text, TRUE, "https://www.renesas.com/en/products/ptx205r", NULL },
    { "Title", NdefField_Text, FALSE, "", NULL },
    { "Language", NdefField_Text, FALSE, "en", NULL },
};

static const NdefFieldDef PHONE_FIELDS[] = {
    { "Number", NdefField_Text, TRUE, "", NULL },
};

static const NdefFieldDef EMAIL_FIELDS[] = {
    { "Address", NdefField_Text, TRUE, "", NULL },
    { "Subject", NdefField_Text, FALSE, "", NULL },
    { "Body", NdefField_Text, FALSE, "", NULL },
};

static const NdefFieldDef SMS_FIELDS[] = {
    { "Number", NdefField_Text, TRUE, "", NULL },
    { "Message", NdefField_Text, FALSE, "", NULL },
};

static const NdefFieldDef GEO_FIELDS[] = {
    { "Latitude", NdefField_Text, TRUE, "", NULL },
    { "Longitude", NdefField_Text, TRUE, "", NULL },
};

static const NdefFieldDef VCARD_FIELDS[] = {
    { "Name", NdefField_Text, TRUE, "", NULL },
    { "Phone", NdefField_Text, FALSE, "", NULL },
    { "Email", NdefField_Text, FALSE, "", NULL },
    { "Organization", NdefField_Text, FALSE, "", NULL },
};

static const NdefFieldDef MIME_FIELDS[] = {
    { "MIME type", NdefField_Text, TRUE, "text/plain", NULL },
    { "Content", NdefField_Text, TRUE, "", NULL },
};

static const NdefFieldDef WIFI_FIELDS[] = {
    { "SSID", NdefField_Text, TRUE, "", NULL },
    { "Authentication", NdefField_Choice, TRUE, NULL, WIFI_AUTH_CHOICES },
    { "Network key", NdefField_Text, FALSE, "", NULL },
};

static const NdefFieldDef ANDROID_APP_FIELDS[] = {
    { "Package name", NdefField_Text, TRUE, "", NULL },
};

#define RECORD_TYPE(name, fields, encode) { name, fields, G_N_ELEMENTS(fields), encode }

static const NdefRecordTypeDef RECORD_TYPES[] = {
    RECORD_TYPE("URI",          URI_FIELDS,          encodeUri),
    RECORD_TYPE("Text",         TEXT_FIELDS,         encodeText),
    RECORD_TYPE("Smart Poster", SMART_POSTER_FIELDS, encodeSmartPoster),
    RECORD_TYPE("Phone",        PHONE_FIELDS,        encodePhone),
    RECORD_TYPE("Email",        EMAIL_FIELDS,        encodeEmail),
    RECORD_TYPE("SMS",          SMS_FIELDS,          encodeSms),
    RECORD_TYPE("Geo location", GEO_FIELDS,          encodeGeo),
    RECORD_TYPE("vCard",        VCARD_FIELDS,        encodeVCard),
    RECORD_TYPE("MIME",         MIME_FIELDS,         encodeMime),
    RECORD_TYPE("Wi-Fi",        WIFI_FIELDS,         encodeWifi),
    RECORD_TYPE("Android app",  ANDROID_APP_FIELDS,  encodeAndroidApp),
};

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

const NdefRecordTypeDef *ptx_ExampleNdef_GetRecordTypes(guint *count)
{
    *count = G_N_ELEMENTS(RECORD_TYPES);
    return RECORD_TYPES;
}

const char *ptx_ExampleNdef_GetUriPrefix(uint8_t code)
{
    return (code < G_N_ELEMENTS(URI_PREFIXES)) ? URI_PREFIXES[code] : NULL;
}

NdefRecordData *ptx_ExampleNdef_EncodeRecord(const NdefRecordTypeDef *def, const gchar * const *values, GError **error)
{
    for (guint i = 0; i < def->nFields; ++i)
    {
        const NdefFieldDef *field = &def->fields[i];

        if (field->required && isBlank(values[i])) {
            g_set_error(error, ndefErrorQuark(), 0, "%s: \"%s\" is required.", def->name, field->label);
            return NULL;
        }

        if ((NdefField_Choice == field->kind) && !g_strv_contains((const gchar * const *)field->choices, values[i])) {
            g_set_error(error, ndefErrorQuark(), 0, "%s: invalid choice for \"%s\".", def->name, field->label);
            return NULL;
        }
    }

    NdefRecordData *data = recordDataNew();
    GError *encodeError = NULL;

    if (!def->encode(values, data, &encodeError)) {
        g_set_error(error, ndefErrorQuark(), 0, "%s: %s", def->name, encodeError->message);
        g_error_free(encodeError);
        ptx_ExampleNdef_RecordDataFree(data);
        return NULL;
    }

    if (data->type->len > 255u) {
        g_set_error(error, ndefErrorQuark(), 0, "%s: record type is too long.", def->name);
        ptx_ExampleNdef_RecordDataFree(data);
        return NULL;
    }

    return data;
}

void ptx_ExampleNdef_RecordDataFree(NdefRecordData *data)
{
    if (NULL != data) {
        g_byte_array_unref(data->type);
        g_byte_array_unref(data->payload);
        g_free(data->summary);
        g_free(data);
    }
}

gboolean ptx_ExampleNdef_BuildMessage(GPtrArray *records, uint8_t *buffer, uint32_t *len, GError **error)
{
    if ((NULL == records) || (0u == records->len)) {
        g_set_error_literal(error, ndefErrorQuark(), 0, "No records to write.");
        return FALSE;
    }

    /* Check the size of the encoded message up-front: header, type length, payload length (1 or 4 bytes), type, payload. */
    uint64_t total = 0;
    for (guint i = 0; i < records->len; ++i) {
        const NdefRecordData *data = g_ptr_array_index(records, i);
        total += 2u + ((data->payload->len > 255u) ? 4u : 1u) + data->type->len + data->payload->len;
    }

    if (total > *len) {
        g_set_error(error, ndefErrorQuark(), 0, "NDEF message too large: %" G_GUINT64_FORMAT " bytes (max. %u bytes).", total, *len);
        return FALSE;
    }

    ptx_NDEFRecord_t *ndefRecords = g_new0(ptx_NDEFRecord_t, records->len);
    ptx_Status_t status = PTX_STATUS_SUCCESS;

    for (guint i = 0; (i < records->len) && (PTX_STATUS_SUCCESS == status); ++i) {
        NdefRecordData *data = g_ptr_array_index(records, i);
        status = ptx_NDEFRecord_Init(&ndefRecords[i]);
        if (PTX_STATUS_SUCCESS == status) {
            status = ptx_NDEFRecord_Create(&ndefRecords[i], (ptx_NDEFRecord_TNF_t)data->tnf,
                                           (data->type->len > 0u) ? data->type->data : NULL, (uint8_t)data->type->len,
                                           NULL, 0,
                                           (data->payload->len > 0u) ? data->payload->data : NULL, data->payload->len);
        }
    }

    if (PTX_STATUS_SUCCESS == status) {
        status = ptx_NDEFMessage_Create(ndefRecords, records->len, buffer, len);
    }

    g_free(ndefRecords);

    if (PTX_STATUS_SUCCESS != status) {
        g_set_error(error, ndefErrorQuark(), 0, "Creating the NDEF message failed (0x%04X).", status);
        return FALSE;
    }

    return TRUE;
}
