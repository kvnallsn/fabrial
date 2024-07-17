<a name="readme-top"></a>


<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <img src="images/logo.png" alt="Logo" width="80" height="80">

  <h3 align="center">Fabrial</h3>

  <p align="center">
    A virtual machine health and status daemon
    <br />
    <a href="https://github.com/kvnallsn/fabrial/issues/new?labels=bug">Report Bug</a>
    ·
    <a href="https://github.com/kvnallsn/fabrial/issues/new?labels=enhancement">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#troubleshooting">Troubleshooting</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project
Fabrial is a virtual machine communication daemon supporting [oathgate](https://github.com/kvnallsn/oathgate) virtual machines. It provides a pseudo-terminal (PTY) bound to a vhost-vsock socket.

<p align="right">(<a href="#readme-top">back to top</a>)</p>


### Built With

[![C][c-shield]][c-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- GETTING STARTED -->
## Getting Started

Fabrial is built using only the C stdlib targeting Linux.  The only requirement is a C compiler.

### Prerequisites

* GCC
  ```sh
  sudo apt install build-essential
  ```

### Building

1. Clone the repo
   ```sh
   git clone https://github.com/kvnallsn/fabrial.git
   ```

2. Build the executables
   ```sh
   make
   ```

   The `fabrial` executable will be located in the `bin/` folder.

### Installation

The binary must be installed inside a virtual machine.  For convenience, an OpenRC init script is provided in the `dist/openrc` directory.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

Running `fabrial` is simple, it does not take any arguments (yet). Running this program outside a virtual machine is unsupported.

1. Start a virtual machine with a `vhost-vsock` device.  For example, include the following arguments when starting a qemu virtual machine:
   ```sh
   qemu-system-x86_64 \
   -other-qemu-args... \
   -device vhost-vsock-pci,guest-cid=4
   ```

2. Inside the virtual machine, start the daemon
   ```sh
   ./bin/fabrial
   ```

<!-- ROADMAP -->
## Roadmap

- [ ] Add arguments for port and spawned program
- [ ] Add ability to "port forward" to locally-bound TCP/UDP sockets

See the [open issues](https://github.com/kvnallsn/fabrial/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- TROUBLESHOOTING -->
## Troubleshooting

If using the OpenRC init script, stdout/stderr will be logged to `/var/log/fabrial.log` and `/var/log/fabrial.err` respectively.

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/kvnallsn/fabrialstyle=for-the-badge
[contributors-url]: https://github.com/kvnallsn/fabrial/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/kvnallsn/fabrialstyle=for-the-badge
[forks-url]: https://github.com/kvnallsn/fabrial/network/members
[stars-shield]: https://img.shields.io/github/stars/kvnallsn/fabrialstyle=for-the-badge
[stars-url]: https://github.com/kvnallsn/fabrial/stargazers
[issues-shield]: https://img.shields.io/github/issues/kvnallsn/fabrial?style=for-the-badge
[issues-url]: https://github.com/kvnallsn/fabrial/issues
[license-shield]: https://img.shields.io/github/license/kvnallsn/fabrial?style=for-the-badge
[license-url]: https://github.com/kvnallsn/fabrial/blob/master/LICENSE.txt
[c-shield]: https://img.shields.io/badge/gcc-blue?style=for-the-badge&logo=C&logoColor=white
[c-url]: https://en.cppreference.com/w/c
