# インストールメモ
<div style="text-align: right;">
3/9/2021<br>
Akihiro SHIMIZU
</div>

## 必要モジュールとそのパッケージ名

- Cairo
~~~
    Debian:   libcairo2
    FreeBSD:  cairo
    Homebrew: cairo
~~~

- Pango
~~~
    Debian:   libpango-1.0, libpango1.0-dev
    FreeBSD:  pango
    Homebrew: pango
~~~
- pkg-config
~~~
    Debian:   pkg-config
    FreeBSD:  pkgconf
    Homebrew: pkg-config
~~~
- GNU make
~~~
    FreeBSD:  gmake
~~~

- IPAfont ... 本文のデフォルトフォントがIPAGothicである為
~~~
    Debian:   fonts-ipafont
    FreeBSD:  ja-font-ipa
    Homebrew: IPAのwebからダウンロードしてOSXからインストール、
          又は cask-fonts/font-ipafont
~~~

## インストール手順

1. 上記のモジュールを`apt/pkg/brew`等でインストール
2. Makefileをチェックし、必要に応じて編集。<br>
   例：FreeBSDでは `MANDIR` を以下のように変更：<br>
   `MANDIR = /usr/local/man`
3. GNU make で以下の通り実行：<br>
   `$ make && sudo make install`


## 日本語フォントについて

Pangoの制限により、使用可能なフォントはフォント名（family name）が半角アル
ファベット、数字、マイナス、空白のみ（正規表現で `[A-Za-z0-9\ \-]+` ）である
物のみのようである。ソースに同梱されているスクリプト `font_candidate.sh` を
用いて

`$ font_candidate.sh ja`

とすれば、候補のフォント名が表示される。

一般的と思われるフォントのうち、上記条件に当てはまる上に全角：半角＝２：１
の幅である物を以下に示す。これらが使える事はMacOS(Mojave)にて確認済み。


|フォント名 | Family name |
|:--:|:--:|
|IPAゴシック| `IPAGothic` |
|IPA明朝    | `IPAMincho` |
|MS明朝     | `MS Mincho` |
|MSゴシック | `MS Gothic` |
|HG明朝E    | `HGMinchoE` |
|HGゴシックE| `HGGothicE` |


