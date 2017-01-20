{
  "targets": [
    {
      "target_name": "judger",
      "sources": [
        "./src/judger.cc",
        "./src/runner.cc",
        "./src/watcher.cc",
        "./src/common.cc",
        "./src/third_party/uuid4/src/uuid4.c"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}