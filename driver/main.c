#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "trie.h"
#include "wordlist.h"
#include <linux/hid.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb/input.h>
#include <uapi/linux/input-event-codes.h>

const char *keycode_map[] = {"KEY_RESERVED",
                             "KEY_ESC",
                             "KEY_1",
                             "KEY_2",
                             "KEY_3",
                             "KEY_4",
                             "KEY_5",
                             "KEY_6",
                             "KEY_7",
                             "KEY_8",
                             "KEY_9",
                             "KEY_0",
                             "KEY_MINUS",
                             "KEY_EQUAL",
                             "KEY_BACKSPACE",
                             "KEY_TAB",
                             "KEY_Q",
                             "KEY_W",
                             "KEY_E",
                             "KEY_R",
                             "KEY_T",
                             "KEY_Y",
                             "KEY_U",
                             "KEY_I",
                             "KEY_O",
                             "KEY_P",
                             "KEY_LEFTBRACE",
                             "KEY_RIGHTBRACE",
                             "KEY_ENTER",
                             "KEY_LEFTCTRL",
                             "KEY_A",
                             "KEY_S",
                             "KEY_D",
                             "KEY_F",
                             "KEY_G",
                             "KEY_H",
                             "KEY_J",
                             "KEY_K",
                             "KEY_L",
                             "KEY_SEMICOLON",
                             "KEY_APOSTROPHE",
                             "KEY_GRAVE",
                             "KEY_LEFTSHIFT",
                             "KEY_BACKSLASH",
                             "KEY_Z",
                             "KEY_X",
                             "KEY_C",
                             "KEY_V",
                             "KEY_B",
                             "KEY_N",
                             "KEY_M",
                             "KEY_COMMA",
                             "KEY_DOT",
                             "KEY_SLASH",
                             "KEY_RIGHTSHIFT",
                             "KEY_KPASTERISK",
                             "KEY_LEFTALT",
                             "KEY_SPACE",
                             "KEY_CAPSLOCK",
                             "KEY_F1",
                             "KEY_F2",
                             "KEY_F3",
                             "KEY_F4",
                             "KEY_F5",
                             "KEY_F6",
                             "KEY_F7",
                             "KEY_F8",
                             "KEY_F9",
                             "KEY_F10",
                             "KEY_NUMLOCK",
                             "KEY_SCROLLLOCK",
                             "KEY_KP7",
                             "KEY_KP8",
                             "KEY_KP9",
                             "KEY_KPMINUS",
                             "KEY_KP4",
                             "KEY_KP5",
                             "KEY_KP6",
                             "KEY_KPPLUS",
                             "KEY_KP1",
                             "KEY_KP2",
                             "KEY_KP3",
                             "KEY_KP0",
                             "KEY_KPDOT",
                             "KEY_ZENKAKUHANKAK",
                             "KEY_102ND",
                             "KEY_F11",
                             "KEY_F12",
                             "KEY_RO",
                             "KEY_KATAKANA",
                             "KEY_HIRAGANA",
                             "KEY_HENKAN",
                             "KEY_KATAKANAHIRAGAN",
                             "KEY_MUHENKAN",
                             "KEY_KPJPCOMMA",
                             "KEY_KPENTER",
                             "KEY_RIGHTCTRL",
                             "KEY_KPSLASH",
                             "KEY_SYSRQ",
                             "KEY_RIGHTALT",
                             "KEY_LINEFEED",
                             "KEY_HOME",
                             "KEY_UP",
                             "KEY_PAGEUP",
                             "KEY_LEFT",
                             "KEY_RIGHT",
                             "KEY_END",
                             "KEY_DOWN",
                             "KEY_PAGEDOWN",
                             "KEY_INSERT",
                             "KEY_DELETE",
                             "KEY_MACRO",
                             "KEY_MUTE",
                             "KEY_VOLUMEDOWN",
                             "KEY_VOLUMEUP",
                             "KEY_POWER",
                             "KEY_KPEQUAL",
                             "KEY_KPPLUSMINUS",
                             "KEY_PAUSE",
                             "KEY_SCALE",
                             "KEY_KPCOMMA",
                             "KEY_HANGEUL",
                             "KEY_HANJA",
                             "KEY_YEN",
                             "KEY_LEFTMETA",
                             "KEY_RIGHTMETA",
                             "KEY_COMPOSE",
                             "KEY_STOP",
                             "KEY_AGAIN",
                             "KEY_PROPS",
                             "KEY_UNDO",
                             "KEY_FRONT",
                             "KEY_COPY",
                             "KEY_OPEN",
                             "KEY_PASTE",
                             "KEY_FIND",
                             "KEY_CUT",
                             "KEY_HELP",
                             "KEY_MENU",
                             "KEY_CALC",
                             "KEY_SETUP",
                             "KEY_SLEEP",
                             "KEY_WAKEUP",
                             "KEY_FILE",
                             "KEY_SENDFILE",
                             "KEY_DELETEFILE",
                             "KEY_XFER",
                             "KEY_PROG1",
                             "KEY_PROG2",
                             "KEY_WWW",
                             "KEY_MSDOS",
                             "KEY_COFFEE",
                             "KEY_ROTATE_DISPLA",
                             "KEY_CYCLEWINDOW",
                             "KEY_MAIL",
                             "KEY_BOOKMARKS",
                             "KEY_COMPUTER",
                             "KEY_BACK",
                             "KEY_FORWARD",
                             "KEY_CLOSECD",
                             "KEY_EJECTCD",
                             "KEY_EJECTCLOSEC",
                             "KEY_NEXTSONG",
                             "KEY_PLAYPAUSE",
                             "KEY_PREVIOUSSON",
                             "KEY_STOPCD",
                             "KEY_RECORD",
                             "KEY_REWIND",
                             "KEY_PHONE",
                             "KEY_ISO",
                             "KEY_CONFIG",
                             "KEY_HOMEPAGE",
                             "KEY_REFRESH",
                             "KEY_EXIT",
                             "KEY_MOVE",
                             "KEY_EDIT",
                             "KEY_SCROLLUP",
                             "KEY_SCROLLDOWN",
                             "KEY_KPLEFTPAREN",
                             "KEY_KPRIGHTPARE",
                             "KEY_NEW",
                             "KEY_REDO",
                             "KEY_F13",
                             "KEY_F14",
                             "KEY_F15",
                             "KEY_F16",
                             "KEY_F17",
                             "KEY_F18",
                             "KEY_F19",
                             "KEY_F20",
                             "KEY_F21",
                             "KEY_F22",
                             "KEY_F23",
                             "KEY_F24",
                             "KEY_PLAYCD",
                             "KEY_PAUSECD",
                             "KEY_PROG3",
                             "KEY_PROG4",
                             "KEY_ALL_APPLICATION",
                             "KEY_SUSPEND",
                             "KEY_CLOSE",
                             "KEY_PLAY",
                             "KEY_FASTFORWARD",
                             "KEY_BASSBOOST",
                             "KEY_PRINT",
                             "KEY_HP",
                             "KEY_CAMERA",
                             "KEY_SOUND",
                             "KEY_QUESTION",
                             "KEY_EMAIL",
                             "KEY_CHAT",
                             "KEY_SEARCH",
                             "KEY_CONNECT",
                             "KEY_FINANCE",
                             "KEY_SPORT",
                             "KEY_SHOP",
                             "KEY_ALTERASE",
                             "KEY_CANCEL",
                             "KEY_BRIGHTNESSDOW",
                             "KEY_BRIGHTNESSU",
                             "KEY_MEDIA",
                             "KEY_SWITCHVIDEOMOD",
                             "KEY_KBDILLUMTOGGL",
                             "KEY_KBDILLUMDOW",
                             "KEY_KBDILLUMUP",
                             "KEY_SEND",
                             "KEY_REPLY",
                             "KEY_FORWARDMAIL",
                             "KEY_SAVE",
                             "KEY_DOCUMENTS",
                             "KEY_BATTERY",
                             "KEY_BLUETOOTH",
                             "KEY_WLAN",
                             "KEY_UWB",
                             "KEY_UNKNOWN",
                             "KEY_VIDEO_NEXT",
                             "KEY_VIDEO_PREV",
                             "KEY_BRIGHTNESS_CYCL",
                             "KEY_BRIGHTNESS_AUT",
                             "KEY_DISPLAY_OFF",
                             "KEY_WWAN",
                             "KEY_RFKILL",
                             "KEY_MICMUTE"};

// from glibc
size_t strlcpy(char *__restrict dest, const char *__restrict src, size_t size) {
  size_t src_length = strlen(src);

  if (src_length >= size) {
    if (size > 0) {
      /* Copy the leading portion of the string.  The last
         character is subsequently overwritten with the NUL
         terminator, but the destination size is usually a
         multiple of a small power of two, so writing it twice
         should be more efficient than copying an odd number of
         bytes.  */
      memcpy(dest, src, size);
      dest[size - 1] = '\0';
    }
  } else
    /* Copy the string and its terminating NUL character.  */
    memcpy(dest, src, src_length + 1);
  return src_length;
}

// from my mind
// size_t strlen(const char * str){
//
// }
/*
 * Version Information
 */
#define DRIVER_VERSION ""
#define DRIVER_AUTHOR "FoxMoss"
#define DRIVER_DESC                                                            \
  "USB HID Boot Protocol keyboard driver with builtin T9 typing"
#define DRIVER_LICENSE "GPL"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

static const unsigned char usb_kbd_keycode[256] = {
    0,   0,   0,   0,   30,  48,  46,  32,  18,  33,  34,  35,  23,  36,  37,
    38,  50,  49,  24,  25,  16,  19,  31,  20,  22,  47,  17,  45,  21,  44,
    2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  28,  1,   14,  15,  57,
    12,  13,  26,  27,  43,  43,  39,  40,  41,  51,  52,  53,  58,  59,  60,
    61,  62,  63,  64,  65,  66,  67,  68,  87,  88,  99,  70,  119, 110, 102,
    104, 111, 107, 109, 106, 105, 108, 103, 69,  98,  55,  74,  78,  96,  79,
    80,  81,  75,  76,  77,  71,  72,  73,  82,  83,  86,  127, 116, 117, 183,
    184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 134, 138, 130, 132,
    128, 129, 131, 137, 133, 135, 136, 113, 115, 114, 0,   0,   0,   121, 0,
    89,  93,  124, 92,  94,  95,  0,   0,   0,   122, 123, 90,  91,  85,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   29,
    42,  56,  125, 97,  54,  100, 126, 164, 166, 165, 163, 161, 115, 114, 113,
    150, 158, 159, 128, 136, 177, 178, 176, 142, 152, 173, 140};

/**
 * struct usb_kbd - state of each attached keyboard
 * @dev:	input device associated with this keyboard
 * @usbdev:	usb device associated with this keyboard
 * @old:	data received in the past from the @irq URB representing which
 *		keys were pressed. By comparing with the current list of keys
 *		that are pressed, we are able to see key releases.
 * @irq:	URB for receiving a list of keys that are pressed when a
 *		new key is pressed or a key that was pressed is released.
 * @led:	URB for sending LEDs (e.g. numlock, ...)
 * @newleds:	data that will be sent with the @led URB representing which LEDs
                should be on
 * @name:	Name of the keyboard. @dev's name field points to this buffer
 * @phys:	Physical path of the keyboard. @dev's phys field points to this
 *		buffer
 * @new:	Buffer for the @irq URB
 * @cr:		Control request for @led URB
 * @leds:	Buffer for the @led URB
 * @new_dma:	DMA address for @irq URB
 * @leds_dma:	DMA address for @led URB
 * @leds_lock:	spinlock that protects @leds, @newleds, and @led_urb_submitted
 * @led_urb_submitted: indicates whether @led is in progress, i.e. it has been
 *		submitted and its completion handler has not returned yet
 *		without	resubmitting @led
 */
struct usb_kbd {
  struct input_dev *dev;
  struct usb_device *usbdev;
  unsigned char old[8];
  struct urb *irq, *led;
  unsigned char newleds;
  char name[128];
  char phys[64];

  unsigned char *new;
  struct usb_ctrlrequest *cr;
  unsigned char *leds;
  dma_addr_t new_dma;
  dma_addr_t leds_dma;

  spinlock_t leds_lock;
  bool led_urb_submitted;
  bool mode;
};
// mode == 0 means mode 1, mode == 1 means mode 2.

static struct TrieNode *root = NULL;

char to_lower(char c) {
  if (c >= 'A' && c <= 'Z') {
    return c - 'A' + 'a';
  }
  return c;
}

char *get_word(char *path, size_t index) {
  struct Optional optional = trie_get_node(root, path);
  if (optional.type == OPTIONAL_ERROR)
    return path;

  struct TrieNode *node = optional.data.ptr;
  if (node->children_len == 0)
    return path;

  int no_words = 0;
  for (size_t i = 0; i < node->children_len; i++) {
    if (node->children[i]->type == TRIE_WORD) {
      no_words++;
    }
  }
  if (no_words == 0) {
    return path;
  }

  size_t skipped = 0;
  size_t current_index = 0;
  do {
    if (node->children[current_index]->type == TRIE_WORD) {
      if (skipped < index) {
        skipped++;
        current_index++;
        current_index %= node->children_len;
        continue;
      }
      return node->children[current_index]->data.w.str;
    }
    current_index++;
    current_index %= node->children_len;
  } while (1);
  return path;
}
char kbd_US[128] = {
    0,   27,   '1', '2',  '3',  '4',  '5',  '6', '7',
    '8', '9',  '0', '-',  '=',  '\b', '\t', /* <-- Tab */
    'q', 'w',  'e', 'r',  't',  'y',  'u',  'i', 'o',
    'p', '[',  ']', '\n', 0, /* <-- control key */
    'a', 's',  'd', 'f',  'g',  'h',  'j',  'k', 'l',
    ';', '\'', '`', 0,    '\\', 'z',  'x',  'c', 'v',
    'b', 'n',  'm', ',',  '.',  '/',  0,    '*', 0, /* Alt */
    ' ',                                            /* Space bar */
    0,                                              /* Caps lock */
    0,                                              /* 59 - F1 key ... > */
    0,   0,    0,   0,    0,    0,    0,    0,   0, /* < ... F10 */
    0,                                              /* 69 - Num lock*/
    0,                                              /* Scroll Lock */
    0,                                              /* Home key */
    0,                                              /* Up Arrow */
    0,                                              /* Page Up */
    '-', 0,                                         /* Left Arrow */
    0,   0,                                         /* Right Arrow */
    '+', 0,                                         /* 79 - End key*/
    0,                                              /* Down Arrow */
    0,                                              /* Page Down */
    0,                                              /* Insert Key */
    0,                                              /* Delete Key */
    0,   0,    0,   0,                              /* F11 Key */
    0,                                              /* F12 Key */
    0, /* All other keys are undefined */
};

static char buffer[256];
static size_t index = 0;
static char *str = "";
static size_t cursor = 0;

static void input_report_key_proxy(struct input_dev *dev, unsigned int code,
                                   int value) {
  if (root == NULL) {
    root = trie_load_wordlist();
  }
  if (1 != value) {
    return;
  }
  unsigned int ret_code = code;

  if (ret_code == KEY_KP0) {
    input_report_key(dev, KEY_SPACE, 1);
    input_report_key(dev, KEY_SPACE, 0);
    str = "";
    cursor = 0;
    index = 0;
    return;
  }

  char c = 0;
  size_t last_strlen = strlen(str);

  if (ret_code == KEY_KP7) {
    ret_code = KEY_KP1;
    c = '1';
  } else if (ret_code == KEY_KP8) {
    ret_code = KEY_KP2;
    c = '2';
  } else if (ret_code == KEY_KP9) {
    ret_code = KEY_KP3;
    c = '3';
  } else if (ret_code == KEY_KP4) {
    ret_code = KEY_KP4;
    c = '4';
  } else if (ret_code == KEY_KP5) {
    ret_code = KEY_KP5;
    c = '5';
  } else if (ret_code == KEY_KP6) {
    ret_code = KEY_KP6;
    c = '6';
  } else if (ret_code == KEY_KP1) {
    ret_code = KEY_KP7;
    c = '7';
  } else if (ret_code == KEY_KP2) {
    ret_code = KEY_KP8;
    c = '8';
  } else if (ret_code == KEY_KP3) {
    ret_code = KEY_KP9;
    c = '9';
  } else if (ret_code == KEY_KPASTERISK) {
    index++;
  } else if (ret_code == KEY_KPMINUS) {
    // could probably be buffer[--cursor] but thats bull
    if (cursor != 0) {
      cursor--;
    }
    buffer[cursor] = 0;
  }

  if (c != 0) {
    buffer[cursor] = c;
    buffer[cursor + 1] = 0;
    cursor++;
    cursor %= 256;
  }

  for (size_t i = 0; i < last_strlen; i++) {
    input_report_key(dev, KEY_BACKSPACE, 1);
    input_report_key(dev, KEY_BACKSPACE, 0);
  }
  if (cursor == 0) {
    return;
  }

  char *local_str = get_word(buffer, index);
  printk("buffer at \"%s\" str is \"%s\"\n", buffer, local_str);

  str = local_str;
  do {
    unsigned int keycode = KEY_SEMICOLON;
    for (size_t i = 0; i < 128; i++) {
      if (kbd_US[i] == to_lower(*local_str)) {
        keycode = i;
      }
    }

    input_report_key(dev, keycode, 1);
    input_report_key(dev, keycode, 0);
    local_str++;
  } while (*local_str != 0);

  // printk("sending key %s\n", keycode_map[ret_code]);
  //
  // input_report_key(dev, ret_code, value);
}

static void usb_kbd_irq(struct urb *urb) {

  struct usb_kbd *kbd = urb->context;
  int i;

  switch (urb->status) {
  case 0: /* success */
    break;
  case -ECONNRESET: /* unlink */
  case -ENOENT:
  case -ESHUTDOWN:
    return;
  /* -EPIPE:  should clear the halt */
  default: /* error */
    goto resubmit;
  }

  for (i = 0; i < 8; i++) {
    input_report_key(kbd->dev, usb_kbd_keycode[i + 224], 0);
  }

  for (i = 2; i < 8; i++) {

    if (kbd->old[i] > 3 &&
        memscan(kbd->new + 2, kbd->old[i], 6) == kbd->new + 8) {
      if (usb_kbd_keycode[kbd->old[i]]) {
        input_report_key_proxy(kbd->dev, usb_kbd_keycode[kbd->old[i]], 0);
      } else
        hid_info(urb->dev, "Unknown key (scancode %#x) released.\n",
                 kbd->old[i]);
    }

    if (kbd->new[i] > 3 &&
        memscan(kbd->old + 2, kbd->new[i], 6) == kbd->old + 8) {
      if (usb_kbd_keycode[kbd->new[i]]) {
        input_report_key_proxy(kbd->dev, usb_kbd_keycode[kbd->new[i]], 1);
        printk("got key %s\n", keycode_map[usb_kbd_keycode[kbd->new[i]]]);
      } else
        hid_info(urb->dev, "Unknown key (scancode %#x) pressed.\n",
                 kbd->new[i]);
    }
  }

  input_sync(kbd->dev);

  memcpy(kbd->old, kbd->new, 8);

resubmit:
  i = usb_submit_urb(urb, GFP_ATOMIC);
  if (i)
    hid_err(urb->dev, "can't resubmit intr, %s-%s/input0, status %d",
            kbd->usbdev->bus->bus_name, kbd->usbdev->devpath, i);
}

static int usb_kbd_event(struct input_dev *dev, unsigned int type,
                         unsigned int code, int value) {
  unsigned long flags;
  struct usb_kbd *kbd = input_get_drvdata(dev);

  if (type != EV_LED)
    return -1;

  spin_lock_irqsave(&kbd->leds_lock, flags);
  //
  if (kbd->mode == 0) {
    if (((!!test_bit(LED_NUML, dev->led)) != (*(kbd->leds) & 1)) &&
        ((!!test_bit(LED_CAPSL, dev->led)) == 0)) {
      kbd->mode = 1;
      kbd->newleds = (!!test_bit(LED_KANA, dev->led) << 3) |
                     (!!test_bit(LED_COMPOSE, dev->led) << 3) |
                     (!!test_bit(LED_SCROLLL, dev->led) << 2) | (1 << 1) |
                     (!!test_bit(LED_NUML, dev->led));
      printk("Now change to MODE 2.\n");

    } else {
      kbd->newleds = (!!test_bit(LED_KANA, dev->led) << 3) |
                     (!!test_bit(LED_COMPOSE, dev->led) << 3) |
                     (!!test_bit(LED_SCROLLL, dev->led) << 2) |
                     (!!test_bit(LED_CAPSL, dev->led) << 1) |
                     (!!test_bit(LED_NUML, dev->led));
    }
  } else {
    if ((!!test_bit(LED_NUML, dev->led)) != (*(kbd->leds) & 1)) {
      kbd->mode = 0;
      kbd->newleds = (!!test_bit(LED_KANA, dev->led) << 3) |
                     (!!test_bit(LED_COMPOSE, dev->led) << 3) |
                     (!!test_bit(LED_SCROLLL, dev->led) << 2) |
                     (!!test_bit(LED_CAPSL, dev->led) << 1) |
                     (!!test_bit(LED_NUML, dev->led));
      printk("Now change to MODE 1\n");
    } else {
      kbd->newleds = (!!test_bit(LED_KANA, dev->led) << 3) |
                     (!!test_bit(LED_COMPOSE, dev->led) << 3) |
                     (!!test_bit(LED_SCROLLL, dev->led) << 2) |
                     (!!!test_bit(LED_CAPSL, dev->led) << 1) |
                     (!!test_bit(LED_NUML, dev->led));
    }
  }
  //

  if (kbd->led_urb_submitted) {
    spin_unlock_irqrestore(&kbd->leds_lock, flags);
    return 0;
  }

  if (*(kbd->leds) == kbd->newleds) {
    spin_unlock_irqrestore(&kbd->leds_lock, flags);
    return 0;
  }

  *(kbd->leds) = kbd->newleds;

  kbd->led->dev = kbd->usbdev;
  if (usb_submit_urb(kbd->led, GFP_ATOMIC))
    pr_err("usb_submit_urb(leds) failed\n");
  else
    kbd->led_urb_submitted = true;

  spin_unlock_irqrestore(&kbd->leds_lock, flags);

  return 0;
}

static void usb_kbd_led(struct urb *urb) {
  unsigned long flags;
  struct usb_kbd *kbd = urb->context;
  /*add print*/
  printk(KERN_ALERT "Received a URB from CTRL endpoint.\n");
  /*add print*/

  if (urb->status)
    hid_warn(urb->dev, "led urb status %d received\n", urb->status);

  spin_lock_irqsave(&kbd->leds_lock, flags);

  if (*(kbd->leds) == kbd->newleds) {
    kbd->led_urb_submitted = false;
    spin_unlock_irqrestore(&kbd->leds_lock, flags);
    return;
  }

  *(kbd->leds) = kbd->newleds;

  kbd->led->dev = kbd->usbdev;
  if (usb_submit_urb(kbd->led, GFP_ATOMIC)) {
    hid_err(urb->dev, "usb_submit_urb(leds) failed\n");
    kbd->led_urb_submitted = false;
  }
  spin_unlock_irqrestore(&kbd->leds_lock, flags);
}

static int usb_kbd_open(struct input_dev *dev) {
  struct usb_kbd *kbd = input_get_drvdata(dev);
  /*add print*/
  printk(KERN_ALERT "Just open the USB keyboard device.\n");
  /*add print*/

  kbd->irq->dev = kbd->usbdev;
  if (usb_submit_urb(kbd->irq, GFP_KERNEL))
    return -EIO;

  return 0;
}

static void usb_kbd_close(struct input_dev *dev) {
  struct usb_kbd *kbd = input_get_drvdata(dev);
  /*add print*/
  printk(KERN_ALERT "Just close the USB keyboard device.\n");
  /*add print*/

  usb_kill_urb(kbd->irq);
}

static int usb_kbd_alloc_mem(struct usb_device *dev, struct usb_kbd *kbd) {
  if (!(kbd->irq = usb_alloc_urb(0, GFP_KERNEL)))
    return -1;
  if (!(kbd->led = usb_alloc_urb(0, GFP_KERNEL)))
    return -1;
  if (!(kbd->new = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &kbd->new_dma)))
    return -1;
  if (!(kbd->cr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL)))
    return -1;
  if (!(kbd->leds = usb_alloc_coherent(dev, 1, GFP_ATOMIC, &kbd->leds_dma)))
    return -1;

  return 0;
}

static void usb_kbd_free_mem(struct usb_device *dev, struct usb_kbd *kbd) {
  usb_free_urb(kbd->irq);
  usb_free_urb(kbd->led);
  usb_free_coherent(dev, 8, kbd->new, kbd->new_dma);
  kfree(kbd->cr);
  usb_free_coherent(dev, 1, kbd->leds, kbd->leds_dma);
}

static int usb_kbd_probe(struct usb_interface *iface,
                         const struct usb_device_id *id) {
  struct usb_device *dev = interface_to_usbdev(iface);
  struct usb_host_interface *interface;
  struct usb_endpoint_descriptor *endpoint;
  struct usb_kbd *kbd;
  struct input_dev *input_dev;
  int i, pipe, maxp;
  int error = -ENOMEM;
  /*add print*/
  printk(KERN_ALERT "usbkbd driver probing USB keyboard device...\n");
  /*add print*/

  interface = iface->cur_altsetting;

  if (interface->desc.bNumEndpoints != 1)
    return -ENODEV;

  endpoint = &interface->endpoint[0].desc;
  if (!usb_endpoint_is_int_in(endpoint))
    return -ENODEV;

  pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
  maxp = usb_maxpacket(dev, pipe);

  kbd = kzalloc(sizeof(struct usb_kbd), GFP_KERNEL);
  input_dev = input_allocate_device();
  if (!kbd || !input_dev)
    goto fail1;

  if (usb_kbd_alloc_mem(dev, kbd))
    goto fail2;

  kbd->usbdev = dev;
  kbd->dev = input_dev;
  spin_lock_init(&kbd->leds_lock);

  if (dev->manufacturer)
    strlcpy(kbd->name, dev->manufacturer, sizeof(kbd->name));

  if (dev->product) {
    if (dev->manufacturer)
      strlcat(kbd->name, " ", sizeof(kbd->name));
    strlcat(kbd->name, dev->product, sizeof(kbd->name));
  }

  if (!strlen(kbd->name))
    snprintf(kbd->name, sizeof(kbd->name), "USB HIDBP Keyboard %04x:%04x",
             le16_to_cpu(dev->descriptor.idVendor),
             le16_to_cpu(dev->descriptor.idProduct));

  usb_make_path(dev, kbd->phys, sizeof(kbd->phys));
  strlcat(kbd->phys, "/input0", sizeof(kbd->phys));

  input_dev->name = kbd->name;
  input_dev->phys = kbd->phys;
  usb_to_input_id(dev, &input_dev->id);
  input_dev->dev.parent = &iface->dev;

  input_set_drvdata(input_dev, kbd);

  input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_LED) | BIT_MASK(EV_REP);
  input_dev->ledbit[0] = BIT_MASK(LED_NUML) | BIT_MASK(LED_CAPSL) |
                         BIT_MASK(LED_SCROLLL) | BIT_MASK(LED_COMPOSE) |
                         BIT_MASK(LED_KANA);

  for (i = 0; i < 255; i++)
    set_bit(usb_kbd_keycode[i], input_dev->keybit);
  clear_bit(0, input_dev->keybit);

  input_dev->event = usb_kbd_event;
  input_dev->open = usb_kbd_open;
  input_dev->close = usb_kbd_close;

  usb_fill_int_urb(kbd->irq, dev, pipe, kbd->new, (maxp > 8 ? 8 : maxp),
                   usb_kbd_irq, kbd, endpoint->bInterval);
  kbd->irq->transfer_dma = kbd->new_dma;
  kbd->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

  kbd->cr->bRequestType = USB_TYPE_CLASS | USB_RECIP_INTERFACE;
  kbd->cr->bRequest = 0x09;
  kbd->cr->wValue = cpu_to_le16(0x200);
  kbd->cr->wIndex = cpu_to_le16(interface->desc.bInterfaceNumber);
  kbd->cr->wLength = cpu_to_le16(1);

  usb_fill_control_urb(kbd->led, dev, usb_sndctrlpipe(dev, 0), (void *)kbd->cr,
                       kbd->leds, 1, usb_kbd_led, kbd);
  kbd->led->transfer_dma = kbd->leds_dma;
  kbd->led->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

  error = input_register_device(kbd->dev);
  if (error)
    goto fail2;

  usb_set_intfdata(iface, kbd);
  device_set_wakeup_enable(&dev->dev, 1);
  return 0;

fail2:
  usb_kbd_free_mem(dev, kbd);
fail1:
  input_free_device(input_dev);
  kfree(kbd);
  return error;
}

static void usb_kbd_disconnect(struct usb_interface *intf) {
  struct usb_kbd *kbd = usb_get_intfdata(intf);
  /*add print*/
  printk(KERN_ALERT "Time to say bye to the USB keyboard device..\n");
  /*add print*/
  usb_set_intfdata(intf, NULL);
  if (kbd) {
    usb_kill_urb(kbd->irq);
    input_unregister_device(kbd->dev);
    usb_kill_urb(kbd->led);
    usb_kbd_free_mem(interface_to_usbdev(intf), kbd);
    kfree(kbd);
  }
}

static struct usb_device_id usb_kbd_id_table[] = {
    {USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
                        USB_INTERFACE_PROTOCOL_KEYBOARD)},
    {} /* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, usb_kbd_id_table);

static struct usb_driver usb_kbd_driver = {
    .name = "usbkbd",
    .probe = usb_kbd_probe,
    .disconnect = usb_kbd_disconnect,
    .id_table = usb_kbd_id_table,
};

module_usb_driver(usb_kbd_driver);
