const path = require("path");

module.exports = {
  mode: "production",
  entry: path.resolve(__dirname, "app.mjs"),
  output: {
    path: path.resolve(__dirname, "build"),
    filename: "out.js",
    chunkFilename: "chunks/[name]-[contenthash].js",
    clean: true,
  },
  resolve: {
    extensions: [".mjs", ".js"],
    fullySpecified: false,
  },
  module: {
    rules: [
      {
        test: /\.mjs$/,
        type: "javascript/auto",
      },
    ],
  },
};
